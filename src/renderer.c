#include "renderer.h"
#include <ncurses.h>

void render_game(const GameState *state) {
    clear();

    // Vykreslenie hraníc
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (i == 0 || i == GRID_HEIGHT - 1 || j == 0 || j == GRID_WIDTH - 1) {
                mvprintw(i, j, "#");  // Steny
            }
        }
    }

    // Vykreslenie hadíka
    for (int i = 0; i < state->snake.length; i++) {
        mvprintw(state->snake.body[i].x, state->snake.body[i].y, "O");  // Hadíková časť
    }

    // Vykreslenie ovocia
    mvprintw(state->fruit.x, state->fruit.y, "@");  // Ovocie

    // Zobrazenie skóre
    mvprintw(GRID_HEIGHT, 0, "Score: %d", state->score);

    refresh();  // Aktualizuj obrazovku
}

