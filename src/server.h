#ifndef SERVER_H
#define SERVER_H

#include "game.h"
#include "time.h"
#include <ncurses.h>

typedef struct {
    int socket;
    Game game;
    int game_mode;     // 0 = štandardný, 1 = časový
    int game_time;     // Celkový čas hry (v sekundách) pre časový režim
    time_t start_time; // Čas začiatku hry
    int active_players; // Počet aktívnych hráčov
    time_t last_activity_time; // Čas poslednej aktivity (štandardný režim)
} Server;

typedef struct {
    Server *server; // Ukazovateľ na server
    int client_socket; // Socket klienta
} ClientArgs;

void init_server(Server *server, int port, int game_mode, int game_time);
void *client_handler(void *client_socket);
void start_game(Server *server);
void close_server(Server *server);

#endif
