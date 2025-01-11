#ifndef GAME_H
#define GAME_H

#include "snake.h"
#include <ncurses.h>

#define BOARD_HEIGHT 10
#define BOARD_WIDTH 30
#define MAX_OBSTACLES 5

typedef struct {
    int x;
    int y;
} Obstacle;

// Štruktúra pre herný svet
typedef struct {
    int width;              // Šírka hracej plochy
    int height;             // Výška hracej plochy
    Snake snake;            // Hadík
    int *fruit_x;           // X pozície ovocia
    int *fruit_y;           // Y pozície ovocia
    int num_fruits;         // Počet ovocia
    int num_obstacles;      // Počet prekážok
    int score;              // Skóre
    Obstacle *obstacles; // Dynamické pole prekážok
} Game;

void init_game(Game *game, int width, int height, int num_players, int has_obstacles);
void generate_fruit(Game *game, int num_players);
void generate_game_world(Game *game, int num_players, int has_obstacles);
void generate_obstacles(Game *game);
void draw_game(WINDOW *win, Game *game);
void load_obstacles_from_file(Game *game, const char *filename);
int is_valid_position(Game *game, int x, int y);

#endif
