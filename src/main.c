#include "game.h"
#include "renderer.h"
#include <ncurses.h>
#include <unistd.h>

int main() {
    GameState state;
    initialize_game(&state);

    initscr();
    curs_set(0); // Skrytie kurzora
    noecho();
    nodelay(stdscr, TRUE); // Nespomaľuje čakacie vstupy

    while (state.is_running) {
        render_game(&state); // Vykreslí stav hry

        // Ovládanie smeru pomocou vstupu od používateľa
        int ch = getch();
        if (ch == 'w') state.snake.direction = (Position){-1, 0}; // Hore
        if (ch == 's') state.snake.direction = (Position){1, 0};  // Dole
        if (ch == 'a') state.snake.direction = (Position){0, -1}; // Doľava
        if (ch == 'd') state.snake.direction = (Position){0, 1};  // Doprava
        if (ch == 'q') state.is_running = 0; // Ukonči hru, ak stlačíš 'q'

        update_game(&state); // Aktualizuje stav hry

        usleep(200000); // Spomalenie (200ms medzi iteráciami)
    }


    endwin();
    return 0;
}
