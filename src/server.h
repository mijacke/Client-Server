#ifndef SERVER_H
#define SERVER_H

#include "game.h"  // Uistíme sa, že použijeme správny typ pre `Game`
#include <time.h>
#include <ncurses.h>
#include "snake.h"

#define MAX_PLAYERS 2  // Maximálny počet hráčov v hre

// Definícia štruktúry pre ovocie
typedef struct {
    int x;  // X pozícia ovocia
    int y;  // Y pozícia ovocia
} Fruit;

// Zoznam hráčov
typedef struct {
    Snake snake;
    int client_socket;
    int active;  // Stav hráča (aktívny alebo neaktívny)
} Player;

// Struktúra servera
typedef struct {
    int socket;                // Socket servera
    Game game;                 // Stav hry
    int game_mode;             // 0 = štandardný, 1 = časový
    int game_time;             // Časový limit pre hru
    time_t start_time;         // Čas začiatku hry
    int active_players;        // Počet aktívnych hráčov
    time_t last_activity_time; // Čas poslednej aktivity
} Server;

typedef struct {
    Server *server;          // Ukazovateľ na server
    int client_socket;       // Socket klienta
} ClientArgs;

void init_server(Server *server, int port, int game_mode, int game_time, int num_players);
void *client_handler(void *client_socket);
void start_game(Server *server, int has_obstacles);
void close_server(Server *server);
int check_collision(Snake *snake, Player *players, int num_players, int width, int height, Game *game);


#endif
