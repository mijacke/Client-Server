#include "server.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>

void init_server(Server *server, int port) {
    struct sockaddr_in server_addr;
    server->socket = socket(AF_INET, SOCK_STREAM, 0);  // Vytvorenie socketu
    if (server->socket < 0) {
        perror("Nepodarilo sa vytvoriť socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);  // Nastavenie portu pre server

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

void *client_handler(void *client_socket) {
    int socket = *((int *)client_socket);
    char buffer[1024];

    initscr();
    noecho();
    curs_set(FALSE);
    WINDOW *win = newwin(BOARD_HEIGHT, BOARD_WIDTH, 0, 0);
    keypad(win, TRUE);

    Game game;
    init_game(&game, BOARD_WIDTH, BOARD_HEIGHT); 

    while (1) {
        draw_game(win, &game);  // Kreslí herný stav
        int n = recv(socket, buffer, sizeof(buffer), 0);  // Čítanie príkazov od klienta
        if (n <= 0) break;  // Ak neprijmeme žiadne dáta, ukončíme hru

        move_snake(&game.snake, buffer[0]);  // Pohyb hadíka na základe príkazu
        if (check_fruit_collision(&game)) {
            // Ak hadík zožral ovocie
        }

        if (check_collision(&game.snake, BOARD_WIDTH, BOARD_HEIGHT)) {
            break;  // Kolízia s hranicami
        }

        usleep(200000);  // Pauza na pohyb
    }

    endwin();
    close(socket);
    free(client_socket);
    return NULL;
}

void start_game(Server *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    while (1) {
        int *client_socket = malloc(sizeof(int));  // Alokácia pamäte pre klienta
        *client_socket = accept(server->socket, (struct sockaddr *)&client_addr, &client_len);  // Akceptovanie pripojenia
        if (*client_socket < 0) {
            perror("Chyba pri prijímaní pripojenia");
            continue;
        }

        // Vytvorenie nového vlákna pre každý pripojený klient
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_handler, (void *)client_socket) != 0) {
            perror("Chyba pri vytváraní vlákna");
        }
    }
}

void close_server(Server *server) {
    close(server->socket);  // Uzavretie socketu
}

