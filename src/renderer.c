#include "renderer.h"
#include <ncurses.h>

void draw_board(WINDOW *win, int snake_x, int snake_y, int fruit_x, int fruit_y) {
    for (int i = 0; i < 30; i++) {
        mvwaddch(win, 0, i, '-');  
        mvwaddch(win, 9, i, '-'); 
    }
    for (int i = 0; i < 10; i++) {
        mvwaddch(win, i, 0, '|');  
        mvwaddch(win, i, 29, '|'); 
    }

    mvwaddch(win, snake_y, snake_x, 'O');  // Vykreslí hadíka
    mvwaddch(win, fruit_y, fruit_x, 'F');  // Vykreslí ovocie
    wrefresh(win);  // Aktualizácia obrazovky
}


