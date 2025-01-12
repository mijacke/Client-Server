#ifndef SERVER_H
#define SERVER_H

#include <time.h>
#include "snake.h"
#include "game.h"

#define MAX_PLAYERS 2

/**
 * Štruktúra pre hráča na serveri
 */
typedef struct {
    Snake snake;
    int client_socket;
    int active;   // 1 = hraje, 0 = nepripojený
    int score;
} Player;

/**
 * Štruktúra servera
 */
typedef struct {
    int socket;          // serverový socket
    int game_mode;       // 1=bez prekážok, 2= s prekážkami
    int game_time;       // ak je časový režim (teda s prekazkami)
    time_t start_time;
    int active_players;
    int running;         // 1= beží, 0= koniec

    int max_players;
    Player players[MAX_PLAYERS];
    // Namiesto jedného ovocia, pole (max 2 ovocia) – 2 hráči max
    int fruit_x[MAX_PLAYERS];
    int fruit_y[MAX_PLAYERS];

    // Prekážky
    Obstacle obstacles[MAX_OBSTACLES];
    int num_obstacles;

    int paused;
} Server;

// Funkcie

void init_server(Server *server, int port, int game_mode, int game_time, int max_players);
void start_server(Server *server);
void stop_server(Server *server);
int check_collision(const Snake *snake, const Player *players, int num_players,
                    int width, int height, const Obstacle *obstacles, int num_obstacles);

#endif
