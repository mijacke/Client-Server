#include "server.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <errno.h>
#include <time.h>
#include "snake.h"

#define MAX_PLAYERS 2  // Maximálny počet hráčov v hre

// Dynamické pole pre ovocie, ktoré bude zdieľané medzi všetkými hráčmi
Fruit *fruits = NULL;  // Tento ukazovateľ bude spravovať ovocie pre viacerých hráčov

// Zoznam hráčov
Player players[MAX_PLAYERS];

// Inicializácia servera
void init_server(Server *server, int port, int game_mode, int game_time, int num_players) {
    server->game_mode = game_mode;
    server->game_time = game_time;
    server->start_time = time(NULL);
    server->last_activity_time = server->start_time;
    server->active_players = 0;

    if (game_mode != 1 && game_mode != 2) {
        printf("Neplatný režim hry. Prosím, vyberte 1 alebo 2.\n");
        exit(1);
    }

    fruits = malloc(num_players * sizeof(Fruit));
    if (fruits == NULL) {
        printf("Chyba pri alokovaní pamäte pre ovocie.\n");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server->socket < 0) {
        perror("Nepodarilo sa vytvoriť socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server->socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Chyba pri bindovaní socketu");
        exit(1);
    }

    if (listen(server->socket, 5) < 0) {
        perror("Chyba pri počúvaní pripojení");
        exit(1);
    }

    printf("Server počúva na porte %d...\n", port);

}

// Obsluha pripojeného klienta
void *client_handler(void *args) {
    ClientArgs *client_args = (ClientArgs *)args;
    Server *server = client_args->server;
    int client_socket = client_args->client_socket;
    free(client_args);

    int player_index = -1;

    // Priraď klienta k hráčovi
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!players[i].active) {
            players[i].client_socket = client_socket;
            players[i].active = 1;
            player_index = i;
            break;
        }
    }

    if (player_index == -1) {
        printf("Server plný, hráč sa nemôže pripojiť.\n");
        close(client_socket);
        return NULL;
    }

    // Inicializácia snake pre hráča
    Snake *snake = &players[player_index].snake;
    init_snake(snake, BOARD_WIDTH / 2, BOARD_HEIGHT / 2);

    // Odošleme informácie o hre
    int time_to_send = (server->game_mode == 2) ? server->game_time : 0; // Ak je režim 2 (časový), pošleme čas
    send(client_socket, &time_to_send, sizeof(time_to_send), 0);
    send(client_socket, &server->start_time, sizeof(server->start_time), 0);

    // Odoslanie prekážok klientovi
    if (server->game.num_obstacles > 0) {
        send(client_socket, &server->game.num_obstacles, sizeof(server->game.num_obstacles), 0);
        send(client_socket, server->game.obstacles, server->game.num_obstacles * sizeof(Obstacle), 0);
    } else {
        int no_obstacles = 0;
        send(client_socket, &no_obstacles, sizeof(no_obstacles), 0);
    }

    printf("Generujem ovocie pre počet hráčov: %d\n", server->active_players);
    generate_fruit(&server->game, server->active_players);
    printf("Ovocie vygenerované.\n");

    // Hlavná herná slučka
    while (1) {
        // Čítanie príkazov od klienta
        char buffer[1];
        int n = recv(client_socket, buffer, sizeof(buffer), 0);
        if (n <= 0) {
            printf("Klient odpojený. Ukončujem obsluhu.\n");
            close(client_socket);
            server->active_players--;  // Znížiť počet aktívnych hráčov
            return NULL;
        }

        // Aktualizovanie pozície hadíka na základe príkazu
        move_snake(snake, buffer[0], 0);

        // **Kontrola kolízie**
        if (check_collision(snake, players, server->active_players, BOARD_WIDTH, BOARD_HEIGHT, &server->game)) {
            printf("Hráč %d narazil. Ukončujem hru pre hráča.\n", player_index);
            char end_signal = 'E';  // Signalizácia konca hry
            send(client_socket, &end_signal, sizeof(end_signal), 0);
            break;  // Ukonči hru pre tohto hráča
        }


        usleep(200000);
    }

    // Ukončenie hry pre klienta
    close(client_socket);
    players[player_index].active = 0;
    printf("Hráč %d odpojený.\n", player_index);
    return NULL;
}

// Predpokladám, že sa nachádzate v sekcii, kde začína hra pre viacerých hráčov

void start_game(Server *server, int has_obstacles) {
    // Pred spustením hry inicializujte štruktúru hry
    init_game(&server->game, BOARD_WIDTH, BOARD_HEIGHT, server->active_players, has_obstacles);

    if (has_obstacles && server->game.num_obstacles > 0) {
        printf("Načítané prekážky zo súboru:\n");
        for (int i = 0; i < server->game.num_obstacles; i++) {
            printf("Prekážka %d: (%d, %d)\n", i, server->game.obstacles[i].x, server->game.obstacles[i].y);
        }
    } else if (has_obstacles) {
        // Ak nie sú prekážky načítané, generujeme ich
        generate_obstacles(&server->game);
        printf("Generované nové prekážky:\n");
        for (int i = 0; i < server->game.num_obstacles; i++) {
            printf("Prekážka %d: (%d, %d)\n", i, server->game.obstacles[i].x, server->game.obstacles[i].y);
        }
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        time_t current_time = time(NULL);

        // **Časový režim (s prekážkami)**
        if (server->game_mode == 2) {
            if (difftime(current_time, server->start_time) >= server->game_time) {
                printf("Koniec hry: Vypršal časový limit %d sekúnd.\n", server->game_time);
                break;
            }
        }

        if (server->game_mode == 1 && server->active_players == 0) {
            if (difftime(current_time, server->last_activity_time) >= 10) {
                printf("Hra ukončená: Žiadny hráč sa nepripojil počas 10 sekúnd.\n");
                break;
            }
        }

        int client_socket = accept(server->socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Chyba pri prijímaní pripojenia");
            continue;
        }

        server->active_players++;
        
        ClientArgs *args = malloc(sizeof(ClientArgs));
        args->server = server;
        args->client_socket = client_socket;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, (void *)args);
    }

    close(server->socket);
    printf("Server ukončil svoju činnosť.\n");
}

// Funkcia na uzavretie servera
void close_server(Server *server) {
    if (server->socket > 0) {
        close(server->socket);
        printf("Socket servera bol uzavretý.\n");
    }

    // Uvoľnenie pamäte pre ovocie
    if (fruits != NULL) {
        free(fruits);
        fruits = NULL;
    }
}

int check_collision(Snake *snake, Player *players, int num_players, int width, int height, Game *game) {
    // Kolízia s hranicami
    if (snake->x[0] < 0 || snake->x[0] >= width || snake->y[0] < 0 || snake->y[0] >= height) {
        return 1;  // Kolízia s hranicami
    }

    // Kolízia so sebou samým
    for (int i = 1; i < snake->length; i++) {
        if (snake->x[0] == snake->x[i] && snake->y[0] == snake->y[i]) {
            return 1;  // Kolízia so sebou samým
        }
    }

    // Kolízia s inými hráčmi
    for (int i = 0; i < num_players; i++) {
        Snake *other_snake = &players[i].snake;
        if (other_snake == snake) continue;  // Preskoč svojho hada

        for (int j = 0; j < other_snake->length; j++) {
            if (snake->x[0] == other_snake->x[j] && snake->y[0] == other_snake->y[j]) {
                return 1;  // Kolízia s iným hadom
            }
        }
    }

    // Kolízia s prekážkami
    for (int i = 0; i < game->num_obstacles; i++) {
        if (snake->x[0] == game->obstacles[i].x && snake->y[0] == game->obstacles[i].y) {
            printf("Had narazil do prekážky na pozícii: (%d, %d)\n", game->obstacles[i].x, game->obstacles[i].y);
            return 1;  // Kolízia s prekážkou
        }
    }

    return 0;  // Žiadna kolízia
}
