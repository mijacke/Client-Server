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
    int fruit_x;  
    int fruit_y;  
} Game;


void init_game(Game *game, int width, int height);

void generate_fruit(Game *game);

int check_fruit_collision(Game *game);

void draw_game(WINDOW *win, Game *game);

#endif

