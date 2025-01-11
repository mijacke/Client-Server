#include "snake.h"
#include <ncurses.h>
#include "server.h"

void init_snake(Snake *snake, int start_x, int start_y) {
    snake->x[0] = start_x;
    snake->y[0] = start_y;
    snake->length = 1;
}

void move_snake(Snake *snake, char direction, int grow) {
    // Posuň telo (od konca po začiatok)
    for (int i = snake->length - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }

    // Posuň hlavu podľa smeru
    switch (direction) {
        case 'w': snake->y[0]--; break;  // Hore
        case 's': snake->y[0]++; break;  // Dole
        case 'a': snake->x[0]--; break;  // Vľavo
        case 'd': snake->x[0]++; break;  // Vpravo
    }

    // Wrap-around správanie (presun na opačnú stranu)
    if (snake->x[0] < 0) snake->x[0] = BOARD_WIDTH - 2;  // Zľava na pravú stranu
    if (snake->x[0] >= BOARD_WIDTH - 1) snake->x[0] = 1;  // Sprava na ľavú stranu
    if (snake->y[0] < 0) snake->y[0] = BOARD_HEIGHT - 2;  // Zhora dole
    if (snake->y[0] >= BOARD_HEIGHT - 1) snake->y[0] = 1; // Zdola hore

    // Ak hadík rastie, predĺži sa telo
    if (grow) {
        snake->length++;
    }
}

void draw_snake(WINDOW *win, Snake *snake) {
    for (int i = 0; i < snake->length; i++) {
        mvwaddch(win, snake->y[i], snake->x[i], 'O');  // Každý článok tela
    }
}


