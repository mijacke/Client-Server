#include "renderer.h"
#include <ncurses.h>
#include "game.h"

void draw_board(WINDOW *win, int snake_x, int snake_y, int fruit_x, int fruit_y) {
    werase(win);  // Vyčistí celé okno

    // Vykreslenie hraníc
    for (int i = 0; i < BOARD_WIDTH; i++) {
        mvwaddch(win, 0, i, '-');  // Horná hranica
        mvwaddch(win, BOARD_HEIGHT - 1, i, '-');  // Dolná hranica
    }
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        mvwaddch(win, i, 0, '|');  // Ľavá hranica
        mvwaddch(win, i, BOARD_WIDTH - 1, '|');  // Pravá hranica
    }

    // Vykreslenie ovocia
    mvwaddch(win, fruit_y, fruit_x, 'F');
    wrefresh(win);  // Aktualizácia okna
}



