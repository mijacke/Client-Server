#ifndef GAME_H
#define GAME_H

#include "snake.h"
#include <ncurses.h>

#define BOARD_HEIGHT 10
#define BOARD_WIDTH 30

typedef struct {
    int width;
    int height;
    Snake snake;
    int *fruit_x;  // Dynamické pole pre ovocie
    int *fruit_y;  // Dynamické pole pre ovocie
    int num_fruits;  // Počet ovocia
} Game;

void init_game(Game *game, int width, int height, int num_players);
void generate_fruit(Game *game, int num_players);
int check_fruit_collision(Game *game);
void draw_game(WINDOW *win, Game *game);

#endif
