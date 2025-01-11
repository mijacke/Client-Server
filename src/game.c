#include "game.h"
#include <stdlib.h>
#include <time.h>

void init_game(Game *game, int width, int height, int num_players, int has_obstacles) {
    game->width = width;
    game->height = height;
    game->score = 0; // Nastavte skóre na 0

    init_snake(&game->snake, width / 2, height / 2);  // Inicializácia hadíka

    game->fruit_x = malloc(num_players * sizeof(int));
    game->fruit_y = malloc(num_players * sizeof(int));
    game->obstacles = malloc(MAX_OBSTACLES * sizeof(Obstacle));

    if (!game->fruit_x || !game->fruit_y || !game->obstacles) {
        printf("Chyba pri alokovaní pamäte.\n");
        exit(1);
    }

    game->num_fruits = num_players;  // Nastavte počet ovocia

    if (has_obstacles) {
        load_obstacles_from_file(game, "../src/obstacles.txt");
    } else {
        game->num_obstacles = 0;
    }

    generate_game_world(game, num_players, has_obstacles);  // Generovanie sveta bez prekážok
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

void generate_obstacles(Game *game) {
    for (int i = 0; i < game->num_obstacles; i++) {
        int valid_position = 0;
        while (!valid_position) {
            valid_position = 1;
            game->obstacles[i].x = rand() % (game->width - 2) + 1;
            game->obstacles[i].y = rand() % (game->height - 2) + 1;

            // Kontrola kolízie s hadom
            for (int j = 0; j < game->snake.length; j++) {
                if (game->snake.x[j] == game->obstacles[i].x &&
                    game->snake.y[j] == game->obstacles[i].y) {
                    valid_position = 0;
                    break;
                }
            }

            // Kontrola kolízie s ovocím
            for (int j = 0; j < game->num_fruits; j++) {
                if (game->fruit_x[j] == game->obstacles[i].x &&
                    game->fruit_y[j] == game->obstacles[i].y) {
                    valid_position = 0;
                    break;
                }
            }
        }
    }
}

void generate_fruit(Game *game, int num_players) {
    for (int i = 0; i < num_players; i++) {
        int valid_position = 0;
        while (!valid_position) {
            game->fruit_x[i] = rand() % (game->width - 2) + 1;
            game->fruit_y[i] = rand() % (game->height - 2) + 1;
            valid_position = is_valid_position(game, game->fruit_x[i], game->fruit_y[i]);
        }
    }
}

int is_valid_position(Game *game, int x, int y) {
    // Kolízia s prekážkami
    for (int i = 0; i < game->num_obstacles; i++) {
        if (game->obstacles[i].x == x && game->obstacles[i].y == y) {
            return 0; // Neplatná pozícia
        }
    }

    // Kolízia s hadíkom
    for (int i = 0; i < game->snake.length; i++) {
        if (game->snake.x[i] == x && game->snake.y[i] == y) {
            return 0; // Neplatná pozícia
        }
    }

    return 1; // Platná pozícia
}

void load_obstacles_from_file(Game *game, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Chyba: Nepodarilo sa otvoriť súbor");
        exit(1);
    }

    int count = 0;
    while (fscanf(file, "%d %d", &game->obstacles[count].x, &game->obstacles[count].y) == 2) {
        printf("Načítaná prekážka: (%d, %d)\n", game->obstacles[count].x, game->obstacles[count].y);
        count++;
        if (count >= MAX_OBSTACLES) break; // Zabráň prekročeniu limitu
    }
    fclose(file);
    game->num_obstacles = count; // Aktualizuj počet načítaných prekážok
}

void draw_game(WINDOW *win, Game *game) {
    //draw_snake(win, &game->snake);

    // Zobrazenie ovocia
    for (int i = 0; i < game->num_fruits; i++) {
        mvwaddch(win, game->fruit_y[i], game->fruit_x[i], 'F');
    }

    // Zobrazenie prekážok
    for (int i = 0; i < game->num_obstacles; i++) {
        mvwaddch(win, game->obstacles[i].y, game->obstacles[i].x, '#');
        printf("Prekážka vykreslená na pozícii (%d, %d)\n", game->obstacles[i].x, game->obstacles[i].y);
    }

    wrefresh(win); // Aktualizácia okna
}
