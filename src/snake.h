#ifndef SNAKE_H
#define SNAKE_H

#define BOARD_WIDTH 30
#define BOARD_HEIGHT 10

// Maximálna dĺžka hadíka pre istotu
#define MAX_SNAKE_LENGTH (BOARD_WIDTH * BOARD_HEIGHT)

/**
 * Štruktúra pre hadíka
 */
typedef struct {
    int x[MAX_SNAKE_LENGTH];  // X súradnice segmentov hadíka
    int y[MAX_SNAKE_LENGTH];  // Y súradnice segmentov hadíka
    int length;               // dĺžka hadíka
    char last_direction;      // naposledy stlačený smer (w,a,s,d)
} Snake;

/**
 * Inicializuje hadíka na danom mieste.
 */
void init_snake(Snake *snake, int start_x, int start_y);

/**
 * Posunie hadíka podľa last_direction; ak je “wrapping”,
 * tak pri vyjdení z mapy sa objaví na druhej strane.
 */
void move_snake(Snake *snake, int width, int height);

/**
 * Zvýši dĺžku hadíka o 1
 */
void grow_snake(Snake *snake);

#endif
