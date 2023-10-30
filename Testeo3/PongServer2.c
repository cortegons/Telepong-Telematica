#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>


struct User {
    char username[50];
    char password[50];
    struct User* next;
};

struct Player {
    int socket;
    char username[50];
    char password[50];  // Agregar el campo password
    int score;
    int best_score;
    int paddle_position;
    int lives;
    struct Game* game;
    struct User* user; // Referencia al usuario autenticado
    // Otros campos necesarios para el jugador
};

struct Ball {
    double x_position;
    double y_position;
    double x_velocity;
    double y_velocity;
    // Agregar otros campos necesarios para la pelota
};

struct Game {
    int id;
    struct Player players[2];
    int score_player1;
    int score_player2;
    struct Ball ball;
    int game_running;
    time_t start_time; // Nuevo campo para el temporizador del juego
    // Agregar otros campos adicionales necesarios para el juego
};

// Estructura que contiene los argumentos del controlador de cliente
struct ClientHandlerArgs {
    struct Player* player;
    struct User* user_list;
    struct Game* game; // Agregar referencia al juego
};

// Prototipos de funciones
void reset_ball(struct Game* game);
int check_paddle_collision(struct Player* player, struct Ball* ball);
void increase_ball_speed(struct Ball* ball);
void change_ball_direction(struct Ball* ball, int paddle_position);
void reset_game(struct Game* game);
struct User* authenticate_user(struct Player* player, struct User* user_list) {
    const char* username = player->username;
    const char* password = player->password;

    struct User* current = user_list;

    // Recorre la lista de usuarios para buscar las credenciales proporcionadas
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && strcmp(current->password, password) == 0) {
            return current; // Devuelve el usuario si las credenciales coinciden
        }
        current = current->next;
    }

    return NULL; // No se encontró el usuario con las credenciales proporcionadas
}
void start_new_game(struct Game* game);  // Debes implementar esta función
void update_game(struct Game* game);     // Debes implementar esta función

#define PORT 8080         // Puerto del servidor
#define MAX_CLIENTS 10    // Cantidad máxima de clientes al mismo tiempo
#define PLAYER_HEIGHT 10  // Altura de la paleta del jugador
#define PLAYER_WIDTH 3    // Ancho de la paleta del jugador
#define FIELD_WIDTH 100   // Ancho del campo de juego

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int connected_players = 0; // Contador para el número de jugadores conectados
struct User* user_list = NULL;

// Función para agregar un usuario a la lista
void add_user(const char* username, const char* password) {
    struct User* new_user = (struct User*)malloc(sizeof(struct User));
    if (new_user == NULL) {
        // Manejar el error de memoria no asignada
        return;
    }
    strcpy(new_user->username, username);
    strcpy(new_user->password, password);
    new_user->next = NULL;

    if (user_list == NULL) {
        user_list = new_user;
    } else {
        struct User* current = user_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_user;
    }
}

// Función para autenticar al usuario
struct User* find_user(const char* username, const char* password) {
    struct User* current = user_list;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0 && strcmp(current->password, password) == 0) {
            return current; // Devuelve el usuario autenticado
        }
        current = current->next;
    }
    return NULL; // Usuario no encontrado o credenciales incorrectas
}

void start_new_game(struct Game* game) {
    // Lógica para iniciar un nuevo juego
    // Puedes inicializar las variables de la estructura `Game`, establecer valores iniciales, etc.
    // Agrega la lógica para preparar un nuevo juego.
}

void update_game(struct Game* game) {
    // Lógica para actualizar el estado del juego en cada iteración del bucle del juego.
    // Realiza las actualizaciones del juego como movimiento de la pelota, detección de colisiones, puntajes, etc.
    // Agrega la lógica para actualizar el estado del juego.
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
        reset_ball(game); // Reiniciar la pelota al centro del campo
    } else if (game->ball.x_position <= 0) {
        // El jugador 2 anota un punto, reiniciar la pelota y aumentar su puntaje
        game->score_player2++;
        reset_ball(game); // Reiniciar la pelota al centro del campo
    }

    // Manejar colisiones con las paletas de los jugadores
    for (int i = 0; i < 2; i++) {
        struct Player* player = &game->players[i];

        if (check_paddle_collision(player, &game->ball)) {
            increase_ball_speed(&game->ball);
            change_ball_direction(&game->ball, player->paddle_position);
        }
    }
}

int check_paddle_collision(struct Player* player, struct Ball* ball) {
    // Verificar si la pelota choca con la paleta del jugador
    return (fabs(player->paddle_position - ball->y_position) < PLAYER_HEIGHT &&
            ((ball->x_position <= PLAYER_WIDTH && ball->x_velocity < 0) ||
            (ball->x_position >= (FIELD_WIDTH - PLAYER_WIDTH) && ball->x_velocity > 0)));
}

void increase_ball_speed(struct Ball* ball) {
    // Aumentar la velocidad de la pelota
    ball->x_velocity *= 1.5;
    ball->y_velocity *= 1.5;
}

void change_ball_direction(struct Ball* ball, int paddle_position) {
    // Calcular el ángulo del rebote según la posición del impacto en la paleta
    double relative_intersect = (paddle_position + PLAYER_HEIGHT / 2) - ball->y_position;
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

void handle_game_end(struct Game* game) {
    char waitlistMessage[] = "Esperando a otro jugador para una nueva partida. Por favor, espere.";
    char endMessage[] = "El juego ha terminado. ¿Desea jugar otra partida? (Sí/No): ";
    char response[3];

    // Enviar mensaje de confirmación a cada cliente
    for (int i = 0; i < 2; ++i) {
        if (game->players[i].socket > 0) {
            send(game->players[i].socket, endMessage, strlen(endMessage), 0);

            // Esperar la respuesta del cliente
            recv(game->players[i].socket, response, sizeof(response), 0);

            // Procesar la respuesta del cliente
            if (strcasecmp(response, "Sí") == 0) {
                // El cliente desea jugar otra partida
                send(game->players[i].socket, waitlistMessage, strlen(waitlistMessage), 0);
            } else {
                // El cliente no desea jugar otra partida
                close(game->players[i].socket);
                game->players[i].socket = -1;
            }
        }
    }

    // Verificar si los jugadores desean jugar otra partida
    if (game->players[0].socket == -1 && game->players[1].socket == -1) {
        // Ambos jugadores se han desconectado o no quieren seguir jugando
        // Restablecer el juego para una nueva partida
        reset_game(game);
    }
}

void reset_game(struct Game* game) {
    // Restablecer los puntajes y el estado del juego
    game->score_player1 = 0;
    game->score_player2 = 0;
    game->game_running = 0;

    // Reiniciar la posición de la pelota utilizando la función de reinicio existente
    reset_ball(game);

    // Reiniciar las posiciones iniciales de las paletas de los jugadores
    for (int i = 0; i < 2; ++i) {
        game->players[i].paddle_position = FIELD_WIDTH / 2 - PLAYER_HEIGHT / 2; // Posición central inicial
    }

    // Restaurar el temporizador del juego para determinar la victoria por tiempo
    game->start_time = time(NULL);  // Iniciar el temporizador nuevamente


    // Notificar a los jugadores sobre el reinicio del juego
    char resetMessage[] = "El juego ha sido restablecido. ¡Listos para una nueva partida!";
    for (int i = 0; i < 2; ++i) {
        if (game->players[i].socket > 0) {
            send(game->players[i].socket, resetMessage, strlen(resetMessage), 0);
        }
    }

    // Restaurar el temporizador del juego u otras variables de control si es necesario
    // Reiniciar cualquier otro estado del juego según tus necesidades
}

void reset_ball(struct Game* game) {
    game->ball.x_position = FIELD_WIDTH / 2;
    game->ball.y_position = PLAYER_HEIGHT * 2;
    game->ball.x_velocity = -game->ball.x_velocity; // Reiniciar la pelota en dirección hacia el otro jugador
}

void check_player_win(struct Game* game) {
    // Verificar si se alcanzó el puntaje máximo para ganar
    if (game->score_player1 >= 15 || game->score_player2 >= 15) {
        handle_game_end(game);
    }
}

void* client_handler(void* arg) {
    struct ClientHandlerArgs* handler_args = (struct ClientHandlerArgs*)arg;
    struct Player* player = handler_args->player;
    struct User* user_list = handler_args->user_list;
    struct Game* game = handler_args->game;

    // Mensaje de bienvenida al cliente
    char welcome_message[] = "Bienvenido al juego Pong.\n";
    send(player->socket, welcome_message, strlen(welcome_message), 0);

    char username[50];
    char password[50];

    // Solicitar al cliente que ingrese su nombre de usuario
    send(player->socket, "Ingrese su nombre de usuario: ", 30, 0);
    recv(player->socket, username, sizeof(username), 0);

    // Solicitar al cliente que ingrese su contraseña
    send(player->socket, "Ingrese su contraseña: ", 25, 0);
    recv(player->socket, password, sizeof(password), 0);

    // Realizar la autenticación del usuario
    struct User* authenticated_user = authenticate_user(player, user_list);

    if (authenticated_user != NULL) {
        send(player->socket, "Autenticacion exitosa. Bienvenido.\n", 35, 0);
        player->user = authenticated_user;  // Asignar el usuario autenticado al jugador
    } else {
        send(player->socket, "Autenticacion fallida. Saliendo.\n", 35, 0);
        close(player->socket);
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&mutex);
    connected_players++;
    if (connected_players == 1) {
        pthread_cond_wait(&cond, &mutex);
    } else {
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);

    // Iniciar el juego después de la sincronización
    start_new_game(game);

    //Bucle que maneja a un cliente
    while (1) {
        update_game(game); // Actualizar el estado del juego

        // Envío de información a los clientes
        char game_info[100]; // Tamaño del mensaje para contener información del juego
        sprintf(game_info, "%f %f %d %d", game->ball.x_position, game->ball.y_position, game->players[0].paddle_position, game->players[1].paddle_position);

        for (int i = 0; i < 2; i++) {
            send(game->players[i].socket, game_info, strlen(game_info), 0);
        }

        // Recepción y manejo de acciones de los jugadores
        char buffer[2]; // Tamaño adecuado para los comandos del protocolo
        for (int i = 0; i < 2; i++) {
            int bytes_received = recv(game->players[i].socket, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                // Manejo de desconexión o error de comunicación
                close(game->players[i].socket);
                pthread_exit(NULL);
            }

            // Manejo de las acciones del jugador según el protocolo
            if (strcmp(buffer, "W") == 0) {
                // Mover la paleta hacia arriba respetando los límites del campo
                int new_position = game->players[i].paddle_position - 1;
                if (new_position >= 0) {
                    game->players[i].paddle_position = new_position;
                }
            } else if (strcmp(buffer, "S") == 0) {
                // Mover la paleta hacia abajo respetando los límites del campo
                int new_position = game->players[i].paddle_position + 1;
                if (new_position <= (FIELD_WIDTH - PLAYER_HEIGHT)) {
                    game->players[i].paddle_position = new_position;
                }
            }
        }

        // Actualizar la posición de la pelota y manejar las colisiones
        update_ball_position(game);
        check_player_win(game);

        // Lógica de espera o temporización
        usleep(10000000); // 10 segundos de retraso (en microsegundos)
    }

    return NULL;

}


int main() {
    struct Game game;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    int connected_players = 0;
    struct User* user_list = NULL;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error al crear el socket del servidor");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al vincular el socket del servidor");
        exit(1);
    }

    if (listen(server_socket, 10) == -1) {
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

        struct ClientHandlerArgs args;
        args.player = &player;
        args.user_list = user_list;
        args.game = &game; // Pasar referencia al juego

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, client_handler, (void*)&args) != 0) {
            perror("Error al crear el hilo del cliente");
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}
