#ifndef SNAKE_H
#define SNAKE_H

#include <ncurses.h>

typedef struct {
    int x;  
    int y;  
} Snake;

void init_snake(Snake *snake, int start_x, int start_y);

void move_snake(Snake *snake, char direction);

int check_collision(Snake *snake, int width, int height);

void draw_snake(WINDOW *win, Snake *snake);

#endif

