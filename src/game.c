#include "game.h"
#include <stdlib.h>
#include <time.h>

void init_game(Game *game, int width, int height, int num_players) {
    game->width = width;
    game->height = height;
    init_snake(&game->snake, width / 2, height / 2);
    game->num_fruits = num_players;
    game->fruit_x = malloc(num_players * sizeof(int));
    game->fruit_y = malloc(num_players * sizeof(int));
    generate_game_world(game, num_players, 0);  // Inicializuje bez prekážok
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
    // Inicializácia pamäte pre ovocie, ak ešte nie je alokovaná
    if (game->fruit_x == NULL || game->fruit_y == NULL) {
        game->fruit_x = malloc(num_players * sizeof(int));  // Dynamické alokovanie pre X pozície ovocia
        game->fruit_y = malloc(num_players * sizeof(int));  // Dynamické alokovanie pre Y pozície ovocia

        // Kontrola úspešnej alokácie
        if (game->fruit_x == NULL || game->fruit_y == NULL) {
            printf("Chyba pri alokovaní pamäti pre ovocie\n");
            exit(1);  // Ak sa alokácia nepodarí, ukončiť hru
        }
    }

    // Generovanie ovocia pre každého hráča
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
