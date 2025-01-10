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

void init_server(Server *server, int port, int game_mode, int game_time) {
    server->game_mode = game_mode;

    // Ak je časový režim, nastavíme čas hry, inak predvolená hodnota 10 sekúnd
    server->game_time = (game_mode == 1) ? game_time : 10;

    server->start_time = time(NULL);  // Nastaví čas začiatku hry
    server->last_activity_time = server->start_time;  // Čas poslednej aktivity
    server->active_players = 0;

    struct sockaddr_in server_addr;
    server->socket = socket(AF_INET, SOCK_STREAM, 0);  // Vytvorenie socketu
    if (server->socket < 0) {
        perror("Nepodarilo sa vytvoriť socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // Povolenie opätovného použitia portu

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

void *client_handler(void *args) {
    ClientArgs *client_args = (ClientArgs *)args;
    Server *server = client_args->server;
    int socket = client_args->client_socket;
    free(client_args);

    // Odoslanie údajov o hre klientovi
    int time_to_send = (server->game_mode == 1) ? server->game_time : 0; // Čas iba pre časový režim
    printf("Odosielam čas hry: %d sekúnd, začiatok hry: %ld\n", time_to_send, server->start_time);
    send(socket, &time_to_send, sizeof(time_to_send), 0);
    send(socket, &server->start_time, sizeof(server->start_time), 0);

    // Inicializácia ncurses
    initscr();
    noecho();
    curs_set(FALSE);
    WINDOW *win = newwin(BOARD_HEIGHT, BOARD_WIDTH, 0, 0);
    keypad(win, TRUE);

    // Inicializácia hry
    Game game;
    init_game(&game, BOARD_WIDTH, BOARD_HEIGHT);

    char buffer[1024];

    while (1) {
        draw_game(win, &game);  // Kreslí herný stav

        // Čítanie príkazov od klienta
        int n = recv(socket, buffer, sizeof(buffer), 0);
        if (n <= 0) break;  // Ak neprijmeme žiadne dáta, ukončíme hru

        // Pohyb hadíka na základe príkazu
        move_snake(&game.snake, buffer[0], 0);

        // Kontrola zjedenia ovocia
        if (check_fruit_collision(&game)) {
            move_snake(&game.snake, buffer[0], 1);  // Ak zožerie ovocie, rastie
        }

        // Kontrola kolízie
        if (check_collision(&game.snake, BOARD_WIDTH, BOARD_HEIGHT)) {
            break;  // Ak došlo ku kolízii, ukončíme hru
        }

        usleep(200000);  // Pauza na pohyb
    }

    // Ukončenie ncurses
    endwin();
    close(socket);
    server->active_players--;  // Zníži počet aktívnych hráčov
    printf("Hráč odpojený. Počet aktívnych hráčov: %d\n", server->active_players);
    return NULL;
}

void start_game(Server *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        time_t current_time = time(NULL);

        // Kontrola ukončenia hry v štandardnom režime
        if (server->game_mode == 0) {  // Štandardný režim
            if (server->active_players == 0 && difftime(current_time, server->last_activity_time) >= 10) {
                printf("Hra končí: Žiadny hráč sa nepripojil v priebehu 10 sekúnd.\n");
                break;  // Ukončí hru
            }
        } else if (server->game_mode == 1) {  // Časový režim
            if (difftime(current_time, server->start_time) >= server->game_time) {
                printf("Hra končí: Vypršal čas hry.\n");
                break;  // Ukončí hru
            }
        }

        // Kontrola nových pripojení
        int client_socket = accept(server->socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100000);  // Čas na čakanie, ak nie sú žiadne nové spojenia
                continue;
            }
            perror("Chyba pri prijímaní pripojenia");
            continue;
        }

        server->active_players++;  // Zvýšenie počtu hráčov
        server->last_activity_time = current_time;  // Aktualizácia poslednej aktivity
        printf("Nový hráč pripojený. Počet aktívnych hráčov: %d\n", server->active_players);

        // Vytvorenie štruktúry ClientArgs
        ClientArgs *args = malloc(sizeof(ClientArgs));
        args->server = server;
        args->client_socket = client_socket;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, (void *)args);
    }

    close(server->socket);
    printf("Server ukončil svoju činnosť.\n");
}


void close_server(Server *server) {
    if (server->socket > 0) {
        close(server->socket);  // Uzavretie socketu
        printf("Socket servera bol uzavretý.\n");
    }
}
