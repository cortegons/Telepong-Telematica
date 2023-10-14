#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_CLIENTS 10
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
    // Agrega otros campos adicionales necesarios para el juego
};

// Función para calcular el ángulo de rebote basado en la posición de impacto en la paleta
double calculate_angle(struct Player* player, struct Ball* ball) {
    double relative_position = (ball->y_position - player->paddle_position) / PLAYER_HEIGHT;
    double max_angle = 45.0; // Máximo ángulo permitido (ajusta según tu juego)
    double angle = max_angle * relative_position;
    angle = angle * M_PI / 180.0;
    return angle;
}

// Función para manejar colisiones con paletas
void handle_paddle_collision(struct Player* player, struct Ball* ball) {
    ball->x_velocity *= 1.1;
    ball->y_velocity *= 1.1;
    double angle = calculate_angle(player, ball);
    double new_x_velocity = cos(angle) * ball->x_velocity - sin(angle) * ball->y_velocity;
    double new_y_velocity = sin(angle) * ball->x_velocity + cos(angle) * ball->y_velocity;
    ball->x_velocity = new_x_velocity;
    ball->y_velocity = new_y_velocity;
}

// Función para enviar el estado del juego a los jugadores (implementación requerida)
void send_game_state(struct Game* game) {
    // Implementa la lógica para enviar el estado del juego a los jugadores aquí
}

// Función para procesar los mensajes del cliente (implementación requerida)
void process_client_message(struct Player* player, const char* message) {
    // Implementa la lógica para procesar los mensajes del cliente aquí
}

// Función para actualizar el estado del juego después de una colisión con una paleta
void update_game_state(struct Game* game) {
    // Realiza la lógica de actualización del juego aquí, por ejemplo, mueve la pelota y verifica colisiones

    // Llama a handle_paddle_collision cuando ocurra una colisión con una paleta
    for (int i = 0; i < 2; i++) {
        struct Player* player = &game->players[i];
        if (fabs(player->paddle_position - game->ball.y_position) < PLAYER_HEIGHT &&
            ((game->ball.x_position <= PLAYER_WIDTH && game->ball.x_velocity < 0) ||
             (game->ball.x_position >= (FIELD_WIDTH - PLAYER_WIDTH) && game->ball.x_velocity > 0))) {
            handle_paddle_collision(player, &game->ball);
        }
    }

    // Agrega lógica adicional según tus necesidades

    // Envía el estado del juego a los jugadores
    send_game_state(game);
}

// Función para gestionar el registro y almacenamiento de la información del jugador
void manage_player_info(struct Player* player) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.txt", player->username);
    FILE* file = fopen(filename, "a+");

    if (file == NULL) {
        perror("Error al abrir el archivo del jugador");
        return;
    }

    int previous_best_score = 0;
    fscanf(file, "%d", &previous_best_score);

    if (player->score > previous_best_score) {
        // Actualiza la mejor marca en el archivo
        fseek(file, 0, SEEK_SET);
        fprintf(file, "%d", player->score);
        player->best_score = player->score;
    } else {
        player->best_score = previous_best_score;
    }

    fclose(file);
}

// Función para manejar la comunicación con un cliente
void* client_handler(void* arg) {
    struct Player* player = (struct Player*)arg;
    struct Game* game = player->game; // Accede a la estructura del juego

    // Mensaje de bienvenida o instrucciones iniciales
    char welcome_message[] = "Bienvenido al juego Pong.\n";
    send(player->socket, welcome_message, strlen(welcome_message), 0);

    char buffer[1024];
    int bytes_received;

    while (1) {
        bytes_received = recv(player->socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            close(player->socket);
            pthread_exit(NULL);
        }

        // Procesa el mensaje del cliente
        process_client_message(player, buffer);

        // Actualiza el estado del juego y notifica a los jugadores
        update_game_state(game);
    }

    return NULL;
}

int main() {
    // Inicializa el juego y otras estructuras necesarias
    struct Game game;
    // Inicializa las estructuras de los jugadores, la pelota y otros campos necesarios para el juego
    // ...

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Configura el servidor y el bucle principal (similar a tu código existente)
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
        // Acepta conexiones entrantes y maneja a los clientes en hilos
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Error al aceptar la conexión entrante");
            continue;
        }

        // Crea un nuevo hilo para manejar la comunicación con el cliente
        struct Player player;
        player.socket = client_socket;
        player.game = &game; // Pasa una referencia al juego al jugador

        int* client_socket_ptr = (int*)malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, (void*)&player) != 0) {
            perror("Error al crear el hilo del cliente");
            free(client_socket_ptr);
            close(client_socket);
        }
    }

    // Cierra el socket del servidor (esto no se ejecutará en el bucle infinito)
    close(server_socket);

    return 0;
}
