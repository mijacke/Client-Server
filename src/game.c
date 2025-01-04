#include "game.h"
#include <stdlib.h>
#include <time.h>

void initialize_game(GameState *state) {
    state->snake.length = 1;
    state->snake.body[0] = (Position){GRID_HEIGHT / 2, GRID_WIDTH / 2};
    state->snake.direction = (Position){0, 1};
    state->score = 0;
    state->is_running = 1;

    spawn_fruit(state);
}

void move_snake(GameState *state) {
    for (int i = state->snake.length - 1; i > 0; i--) {
        state->snake.body[i] = state->snake.body[i - 1];
    }

    state->snake.body[0].x += state->snake.direction.x;
    state->snake.body[0].y += state->snake.direction.y;
}

void spawn_fruit(GameState *state) {
    state->fruit = (Position){rand() % GRID_WIDTH, rand() % GRID_HEIGHT};
}

void update_game(GameState *state) {
    move_snake(state);

    // Detekcia kolízií s ovocím
    if (state->snake.body[0].x == state->fruit.x && state->snake.body[0].y == state->fruit.y) {
        state->snake.length++;
        state->score++;
        spawn_fruit(state);  // Vygeneruje nové ovocie
    }

    // Detekcia kolízie so stenami alebo vlastným telom
    if (check_collision(state)) {
        state->is_running = 0;
    }
}

int check_collision(const GameState *state) {
    // Kolízia so stenami
    if (state->snake.body[0].x < 0 || state->snake.body[0].x >= GRID_HEIGHT ||
        state->snake.body[0].y < 0 || state->snake.body[0].y >= GRID_WIDTH) {
        return 1;
    }

    // Kolízia s vlastným telom
    for (int i = 1; i < state->snake.length; i++) {
        if (state->snake.body[0].x == state->snake.body[i].x &&
            state->snake.body[0].y == state->snake.body[i].y) {
            return 1;
        }
    }

    return 0;
}

