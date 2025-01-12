#include "snake.h"
#include <string.h>

void init_snake(Snake *snake, int start_x, int start_y) {
    // Vynulujeme pamäť
    memset(snake, 0, sizeof(Snake));

    // Napr. hadík bude mať 3 články
    snake->length = 3;
    // Segmenty dáme horizontálne od (start_x, start_y) smerom doľava
    for (int i = 0; i < snake->length; i++) {
        snake->x[i] = start_x - i;
        snake->y[i] = start_y;
    }
    snake->last_direction = 'd'; // default smer doprava
}

void move_snake(Snake *snake, int width, int height) {
    // Posunieme telo: od konca po začiatok
    for (int i = snake->length - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }

    // Posunieme hlavu podľa last_direction
    switch (snake->last_direction) {
        case 'w': snake->y[0]--; break; // hore
        case 's': snake->y[0]++; break; // dole
        case 'a': snake->x[0]--; break; // vľavo
        case 'd': snake->x[0]++; break; // vpravo
        default: break;
    }

    // Wrapping správanie
    if (snake->x[0] < 0) snake->x[0] = width - 1;
    if (snake->x[0] >= width) snake->x[0] = 0;
    if (snake->y[0] < 0) snake->y[0] = height - 1;
    if (snake->y[0] >= height) snake->y[0] = 0;
}

void grow_snake(Snake *snake) {
    // Zvýšime dĺžku o 1
    snake->length++;
    if (snake->length > MAX_SNAKE_LENGTH) {
        snake->length = MAX_SNAKE_LENGTH;
    }
}
