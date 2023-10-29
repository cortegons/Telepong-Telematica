#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define PORT 8080         //Puerto del servidor
#define MAX_CLIENTS 10    //Cantidad máxima de clientes al mismo tiempo
#define PLAYER_HEIGHT 10  // Altura de la paleta del jugador
#define PLAYER_WIDTH 3    // Ancho de la paleta del jugador
#define FIELD_WIDTH 100   // Ancho del campo de juego

// Estructura para representar un jugador
struct Player {
    int socket;
    char username[50];
    int score;
    int best_score; // Para almacenar la mejor marca del jugador
    int paddle_position;
    int lives;
    struct Game* game; // Puntero al juego
    // Agrega otros campos necesarios para el jugador
};

// Estructura para representar la pelota del juego
struct Ball {
    double x_position;
    double y_position;
    double x_velocity;
    double y_velocity;
    // Agrega otros campos necesarios para la pelota
};

// Estructura para representar un juego
struct Game {
    int id;
    struct Player players[2];
    int score_player1;
    int score_player2;
    struct Ball ball; // Agrega una instancia de la pelota al juego
    int game_running; // Indicador de si el juego está en ejecución
    // Agrega otros campos adicionales necesarios para el juego
};

// Prototipos de funciones
void start_new_game(struct Game* game);
void update_game(struct Game* game);
void handle_paddle_collision(struct Player* player, struct Ball* ball);
void game_over(struct Game* game);
void handle_game_end(struct Game* game);
void update_ball_position(struct Game* game);


void handle_game_end(struct Game* game) {
    printf("Juego finalizado. Puntuación final:\n");
    printf("Jugador 1: %d\n", game->score_player1);
    printf("Jugador 2: %d\n", game->score_player2);

    char buffer[256];
    printf("¿Desean iniciar un nuevo juego? (Sí/No): ");
    fgets(buffer, sizeof(buffer), stdin);

    if (strcasecmp(buffer, "Sí\n") != 0) {
        for (int i = 0; i < 2; i++) {
            close(game->players[i].socket);
        }
    } else {
        start_new_game(game);
    }
}

// Otras funciones del juego y lógica de control
int connected_players = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void start_new_game(struct Game* game) {
    game->score_player1 = 0;
    game->score_player2 = 0;
    game->ball.x_position = FIELD_WIDTH / 2;
    game->ball.y_position = PLAYER_HEIGHT * 2;
    game->ball.x_velocity = 1.0;
    game->ball.y_velocity = 1.0;
    game->game_running = 1; // Indica que el juego está en ejecución
    // Agregar lógica adicional de inicialización del juego
}

void update_game(struct Game* game) {
    if (game->game_running) {
        // Actualizar la posición de la pelota
        update_ball_position(game);
    }
}

void update_ball_position(struct Game* game) {
    // Actualizar las coordenadas de la pelota según su velocidad
    game->ball.x_position += game->ball.x_velocity;
    game->ball.y_position += game->ball.y_velocity;

    // Verificar y manejar las colisiones con los bordes del campo
    if (game->ball.y_position >= FIELD_WIDTH - 1 || game->ball.y_position <= 0) {
        game->ball.y_velocity = -game->ball.y_velocity; // Invertir la dirección en caso de colisión con bordes superior e inferior
    }

    if (game->ball.x_position >= FIELD_WIDTH - 1) {
        // El jugador 1 anota un punto, reiniciar la pelota y aumentar su puntaje
        game->score_player1++;
        game->ball.x_position = FIELD_WIDTH / 2;
        game->ball.y_position = PLAYER_HEIGHT * 2;
        game->ball.x_velocity = -game->ball.x_velocity; // Reiniciar la pelota en dirección hacia el jugador 1
    } else if (game->ball.x_position <= 0) {
        // El jugador 2 anota un punto, reiniciar la pelota y aumentar su puntaje
        game->score_player2++;
        game->ball.x_position = FIELD_WIDTH / 2;
        game->ball.y_position = PLAYER_HEIGHT * 2;
        game->ball.x_velocity = -game->ball.x_velocity; // Reiniciar la pelota en dirección hacia el jugador 2
    }

     // Manejar colisiones con las paletas de los jugadores
    for (int i = 0; i < 2; i++) {
        struct Player* player = &game->players[i];

        // Determinar si la pelota choca con la paleta del jugador
        if (fabs(player->paddle_position - game->ball.y_position) < PLAYER_HEIGHT &&
            ((game->ball.x_position <= PLAYER_WIDTH && game->ball.x_velocity < 0) ||
             (game->ball.x_position >= (FIELD_WIDTH - PLAYER_WIDTH) && game->ball.x_velocity > 0))) {
            handle_paddle_collision(player, &game->ball);
        }
    }
}

void handle_paddle_collision(struct Player* player, struct Ball* ball) {
    // Aumentar la velocidad de la pelota
    ball->x_velocity *= 1.5;
    ball->y_velocity *= 1.5;

    // Calcular el ángulo del rebote según la posición del impacto en la paleta
    double relative_intersect = (player->paddle_position + PLAYER_HEIGHT / 2) - ball->y_position;
    double normalized_intersect = (relative_intersect / (PLAYER_HEIGHT / 2));
    double bounce_angle = normalized_intersect * (5 * M_PI / 12);  // Ajustar el ángulo de rebote según tu lógica

    // Cambiar la dirección de la pelota basada en el ángulo de rebote
    if (ball->x_velocity > 0) {
        ball->x_velocity = cos(bounce_angle);
    } else {
        ball->x_velocity = -cos(bounce_angle);
    }

    ball->y_velocity = sin(bounce_angle);
}

void game_over(struct Game* game) {
    game->game_running = 0; // Indica que el juego ha terminado
    // Agregar la lógica para manejar el final del juego
    // Esto podría incluir reiniciar el juego, mostrar resultados finales, etc.
}

void* client_handler(void* arg) {
    struct Player* player = (struct Player*)arg;
    struct Game* game = player->game;

    char welcome_message[] = "Bienvenido al juego Pong.\n";
    send(player->socket, welcome_message, strlen(welcome_message), 0);

    char buffer[1024];
    int bytes_received;

    pthread_mutex_lock(&mutex);
    connected_players++;
    if (connected_players == 2) {
        pthread_cond_signal(&cond);
    } else {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    start_new_game(game);

    while (1) {
        bytes_received = recv(player->socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            close(player->socket);
            pthread_exit(NULL);
        } else {
            buffer[bytes_received] = '\0';  // Agregar el terminador nulo para convertirlo en una cadena de caracteres

            // Procesar el comando del cliente
            if (strcmp(buffer, "W") == 0) {
                // Mover la paleta hacia arriba
                if (player->paddle_position > 0) {
                    player->paddle_position--;
                }
            } else if (strcmp(buffer, "S") == 0) {
                // Mover la paleta hacia abajo
                if (player->paddle_position < FIELD_WIDTH - PLAYER_HEIGHT) {
                    player->paddle_position++;
                }
            }

            // Actualizar el juego después de recibir un comando
            update_game(game);
        }
    }

    return NULL;
}

int main() {
    struct Game game;
    // Inicializa las estructuras de los jugadores, la pelota y otros campos necesarios para el juego

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error al crear el socket del servidor");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al vincular el socket del servidor");
        exit(1);
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Error al escuchar conexiones entrantes");
        exit(1);
    }

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Error al aceptar la conexión entrante");
            continue;
        }

        struct Player player;
        player.socket = client_socket;
        player.game = &game;

        int* client_socket_ptr = (int*)malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, (void*)&player) != 0) {
            perror("Error al crear el hilo del cliente");
            free(client_socket_ptr);
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}
