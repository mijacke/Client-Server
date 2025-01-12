#include "renderer.h"
#include "snake.h"

void draw_game_state(WINDOW *win, const GameState *gs) {
    werase(win);

    // Vykresleny okraj
    for (int i = 0; i < BOARD_WIDTH; i++) {
        mvwaddch(win, 0, i, '-');
        mvwaddch(win, BOARD_HEIGHT - 1, i, '-');
    }
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        mvwaddch(win, i, 0, '|');
        mvwaddch(win, i, BOARD_WIDTH - 1, '|');
    }

    // Prekážky
    for (int i = 0; i < gs->num_obstacles; i++) {
        mvwaddch(win, gs->obstacles[i].y, gs->obstacles[i].x, '#');
    }

    for (int f = 0; f < gs->fruit_count; f++) {
        int fx = gs->fruit_x[f];
        int fy = gs->fruit_y[f];
        mvwaddch(win, fy, fx, 'F');
    }

    // Hady
    for (int p = 0; p < gs->num_players; p++) {
        for (int i = 0; i < gs->snake_length[p]; i++) {
            if (i == 0) {
                // hlava
                mvwaddch(win, gs->snake_y[p][i], gs->snake_x[p][i], 'H');
            } else {
                mvwaddch(win, gs->snake_y[p][i], gs->snake_x[p][i], 'o');
            }
        }
    }

    wrefresh(win);
}
