#ifndef SNAKE_H
#define SNAKE_H

#include <ncurses.h>

#define BOARD_WIDTH 30
#define BOARD_HEIGHT 10

typedef struct {
    int x[BOARD_WIDTH * BOARD_HEIGHT];  // X súradnice tela
    int y[BOARD_WIDTH * BOARD_HEIGHT];  // Y súradnice tela
    int length;  // Dĺžka hadíka
} Snake;

void init_snake(Snake *snake, int start_x, int start_y);
void move_snake(Snake *snake, char direction, int grow);
void draw_snake(WINDOW *win, Snake *snake);

#endif
