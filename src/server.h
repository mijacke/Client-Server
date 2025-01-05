#ifndef SERVER_H
#define SERVER_H

#include "game.h"
#include <ncurses.h>

typedef struct {
    int socket;  
    Game game;   
} Server;

void init_server(Server *server, int port);

void *client_handler(void *client_socket);

void start_game(Server *server);

void close_server(Server *server);

#endif

