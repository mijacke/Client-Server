#include "game.h"
#include <stdlib.h>
#include <time.h>

void initialize_game(GameState *state) {
    // Inicializácia hadíka
    state->snake.length = 1; // Had má na začiatku dĺžku 1
    state->snake.body[0] = (Position){GRID_HEIGHT / 2, GRID_WIDTH / 2}; // Stred mriežky
    state->snake.direction = (Position){0, 1}; // Pohyb doprava

    // Inicializácia ovocia
    state->fruit = (Position){rand() % GRID_WIDTH, rand() % GRID_HEIGHT};

    // Skóre a stav hry
    state->score = 0;
    state->is_running = 1; // Hra je aktívna
}


void update_game(GameState *state) {
    // Pohyb tela
    for (int i = state->snake.length - 1; i > 0; i--) {
        state->snake.body[i] = state->snake.body[i - 1];
    }

    // Pohyb hlavy
    state->snake.body[0].x += state->snake.direction.x;
    state->snake.body[0].y += state->snake.direction.y;

    // Logovanie pozície hlavy
    printf("Head position: (%d, %d)\n", state->snake.body[0].x, state->snake.body[0].y);

    // Detekcia kolízie s ovocím
    if (state->snake.body[0].x == state->fruit.x && state->snake.body[0].y == state->fruit.y) {
        state->snake.length++;
        state->score++;
        state->fruit = (Position){rand() % GRID_WIDTH, rand() % GRID_HEIGHT};
        printf("Ate fruit! New length: %d\n", state->snake.length);
    }

    // Kolízie
    if (check_collision(state)) {
        printf("Collision detected! Game over.\n");
        state->is_running = 0;
    }
}


int check_collision(const GameState *state) {
    // Kolízia so stenami
    if (state->snake.body[0].x < 0 || state->snake.body[0].x >= GRID_HEIGHT ||
        state->snake.body[0].y < 0 || state->snake.body[0].y >= GRID_WIDTH) {
        printf("Wall collision! Head position: (%d, %d)\n",
               state->snake.body[0].x, state->snake.body[0].y);
        return 1;
    }

    // Kolízia s vlastným telom
    for (int i = 1; i < state->snake.length; i++) {
        if (state->snake.body[0].x == state->snake.body[i].x &&
            state->snake.body[0].y == state->snake.body[i].y) {
            printf("Body collision! Head: (%d, %d), Body[%d]: (%d, %d)\n",
                   state->snake.body[0].x, state->snake.body[0].y,
                   i, state->snake.body[i].x, state->snake.body[i].y);
            return 1;
        }
    }

    return 0;
}


