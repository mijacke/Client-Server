#include "game.h"
#include <ncurses.h>
#include <stdlib.h>

void init_game(Game *game, int width, int height) {
    game->width = width;
    game->height = height;
    init_snake(&game->snake, width / 2, height / 2);
    generate_fruit(game); 
}

void generate_fruit(Game *game) {
    game->fruit_x = rand() % (game->width - 2) + 1;
    game->fruit_y = rand() % (game->height - 2) + 1;
}

int check_fruit_collision(Game *game) {
    if (game->snake.x == game->fruit_x && game->snake.y == game->fruit_y) {
        generate_fruit(game); 
        return 1;  
    }
    return 0;  
}

void draw_game(WINDOW *win, Game *game) {
    draw_snake(win, &game->snake); 
    mvwaddch(win, game->fruit_y, game->fruit_x, 'F');  
    wrefresh(win);  
}

