#include "game.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>

void init_game(Game *game, int width, int height, int num_players) {
    game->width = width;
    game->height = height;
    init_snake(&game->snake, width / 2, height / 2);
    game->num_fruits = num_players;  // Nastavíme počet ovocia podľa počtu hráčov
    generate_fruit(game, num_players);  // Voláme generovanie ovocia
}

void generate_fruit(Game *game, int num_players) {
    game->fruit_x = realloc(game->fruit_x, num_players * sizeof(int));  // Dynamicky prispôsobené pole pre ovocie
    game->fruit_y = realloc(game->fruit_y, num_players * sizeof(int));

    for (int i = 0; i < num_players; i++) {
        int valid_position = 0;
        while (!valid_position) {
            valid_position = 1;
            game->fruit_x[i] = rand() % (game->width - 2) + 1;  // Náhodné X
            game->fruit_y[i] = rand() % (game->height - 2) + 1; // Náhodné Y

            // Kontrola, či ovocie nie je na mieste hadíka
            for (int j = 0; j < num_players; j++) {
                if (game->snake.x[0] == game->fruit_x[i] && game->snake.y[0] == game->fruit_y[i]) {
                    valid_position = 0;  // Ak sa ovocie zhoduje s hadíkom, generujeme nové
                    break;
                }
            }
        }
    }
}


int check_fruit_collision(Game *game) {
    for (int i = 0; i < game->num_fruits; i++) {
        if (game->snake.x[0] == game->fruit_x[i] && game->snake.y[0] == game->fruit_y[i]) {
            generate_fruit(game, game->num_fruits);  // Vygeneruje nové ovocie
            return 1;  // Kolízia s ovocím
        }
    }
    return 0;  // Bez kolízie
}


void draw_game(WINDOW *win, Game *game) {
    draw_snake(win, &game->snake);

    for (int i = 0; i < game->num_fruits; i++) {
        mvwaddch(win, game->fruit_y[i], game->fruit_x[i], 'F');  // Zobrazenie ovocia
    }

    wrefresh(win);  // Aktualizácia okna
}


