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

    // Umožníme iba počet hráčov, ktorý zadáme pri vytvorení hry
    if (num_players > MAX_PLAYERS) {
        printf("Počet hráčov nemôže presiahnuť %d\n", MAX_PLAYERS);
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
    int time_to_send = (server->game_mode == 1) ? server->game_time : 0;
    send(client_socket, &time_to_send, sizeof(time_to_send), 0);
    send(client_socket, &server->start_time, sizeof(server->start_time), 0);

    // Generovanie ovocia pre oboch hráčov
    generate_fruit(&server->game, server->active_players);  // Voláme generovanie ovocia so správnymi parametrami

    // Hlavná herná slučka
    while (1) {
        // Čítanie príkazov od klienta
        char buffer[1];
        int n = recv(client_socket, buffer, sizeof(buffer), 0);
        if (n <= 0) break;

        // Aktualizovanie pozície hadíka na základe príkazu
        move_snake(snake, buffer[0], 0);

        // Posielanie pozícií všetkých hadíkov všetkým klientom
        for (int i = 0; i < server->active_players; i++) {
            if (players[i].active && players[i].client_socket != client_socket) {
                send(players[i].client_socket, &snake->x[0], sizeof(snake->x[0]), 0);
                send(players[i].client_socket, &snake->y[0], sizeof(snake->y[0]), 0);
            }
        }

        // Kontrola kolízie, generovanie ovocia a pohyb hadíka
        if (check_collision(snake, BOARD_WIDTH, BOARD_HEIGHT)) {
            break;
        }

        // Kontrola, či hadík zožral ovocie
        for (int i = 0; i < server->active_players; i++) {
            if (snake->x[0] == fruits[i].x && snake->y[0] == fruits[i].y) {
                generate_fruit(&server->game, server->active_players);  // Generovanie nového ovocia
                break;
            }
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

void start_game(Server *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int has_obstacles = get_game_mode() == 2;  // Rozhodne podľa výberu hráča

    while (1) {
        int client_socket = accept(server->socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Chyba pri prijímaní pripojenia");
            continue;
        }

        // Vytvorenie nového vlákna pre pripojeného hráča
        ClientArgs *args = malloc(sizeof(ClientArgs));
        args->server = server;
        args->client_socket = client_socket;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, (void *)args);
        server->active_players++;

        // Generovanie herného sveta na základe voľby hráča
        generate_game_world(&server->game, server->active_players, has_obstacles);
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
