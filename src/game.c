#include "game.h"
#include <stdlib.h>
#include <time.h>

void init_game(Game *game, int width, int height, int num_players) {
    game->width = width;
    game->height = height;

    init_snake(&game->snake, width / 2, height / 2);  // Inicializácia hadíka

    game->fruit_x = malloc(num_players * sizeof(int));
    game->fruit_y = malloc(num_players * sizeof(int));

    if (game->fruit_x == NULL || game->fruit_y == NULL) {
        printf("Chyba pri alokovaní pamäti pre ovocie.\n");
        exit(1);
    }

    game->num_fruits = num_players;  // Nastavte počet ovocia
    generate_game_world(game, num_players, 0);  // Generovanie sveta bez prekážok
}

void generate_game_world(Game *game, int num_players, int has_obstacles) {
    game->num_fruits = num_players;
    generate_fruit(game, num_players);  // Vygeneruje ovocie

    if (has_obstacles) {
        game->num_obstacles = rand() % MAX_OBSTACLES + 1;  // Generovanie náhodného počtu prekážok
        generate_obstacles(game);  // Generovanie prekážok
    } else {
        game->num_obstacles = 0;  // Bez prekážok
    }
}

void generate_fruit(Game *game, int num_players) {
    for (int i = 0; i < num_players; i++) {
        int valid_position = 0;
        while (!valid_position) {
            valid_position = 1;
            game->fruit_x[i] = rand() % (game->width - 2) + 1;
            game->fruit_y[i] = rand() % (game->height - 2) + 1;

            // Overenie, či ovocie nekoliduje s hadíkmi
            for (int j = 0; j < game->snake.length; j++) {
                if (game->snake.x[j] == game->fruit_x[i] && game->snake.y[j] == game->fruit_y[i]) {
                    valid_position = 0;  // Kolízia s hadíkom
                    break;
                }
            }
        }
        printf("Ovocie generované na pozícii: (%d, %d)\n", game->fruit_x[i], game->fruit_y[i]);
    }
}

void generate_obstacles(Game *game) {
    for (int i = 0; i < game->num_obstacles; i++) {
        int valid_position = 0;
        while (!valid_position) {
            valid_position = 1;
            game->obstacles[i].x = rand() % (game->width - 2) + 1;
            game->obstacles[i].y = rand() % (game->height - 2) + 1;

            // Skontroluj, či prekážka nie je na mieste hadíka alebo ovocia
            for (int j = 0; j < game->snake.length; j++) {
                if (game->snake.x[j] == game->obstacles[i].x && game->snake.y[j] == game->obstacles[i].y) {
                    valid_position = 0;
                    break;
                }
            }
        }
    }
}

void draw_game(WINDOW *win, Game *game) {
    draw_snake(win, &game->snake);

    // Zobrazenie ovocia
    for (int i = 0; i < game->num_fruits; i++) {
        mvwaddch(win, game->fruit_y[i], game->fruit_x[i], 'F');
    }

    // Zobrazenie prekážok
    for (int i = 0; i < game->num_obstacles; i++) {
        mvwaddch(win, game->obstacles[i].y, game->obstacles[i].x, '#');
    }

    wrefresh(win);  // Aktualizácia okna
}
