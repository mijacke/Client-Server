#include "snake.h"
#include <ncurses.h>

void init_snake(Snake *snake, int start_x, int start_y) {
    snake->x = start_x;
    snake->y = start_y;
}

void move_snake(Snake *snake, char direction) {
    switch (direction) {
        case 'w': 
            snake->y--;
            break;
        case 's': 
            snake->y++;
            break;
        case 'a': 
            snake->x--;
            break;
        case 'd': 
            snake->x++;
            break;
    }
}

int check_collision(Snake *snake, int width, int height) {
    if (snake->x <= 0 || snake->x >= width - 1 || snake->y <= 0 || snake->y >= height - 1) {
        return 1;  // Kolízia s hranicou
    }
    return 0;  // Bez kolízie
}

void draw_snake(WINDOW *win, Snake *snake) {
    mvwaddch(win, snake->y, snake->x, 'O');  // Vykreslí hadíka
}

