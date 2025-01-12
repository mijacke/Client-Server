#ifndef RENDERER_H
#define RENDERER_H

#include <ncurses.h>
#include "game.h"

/**
 * Vykreslí stav hry do daného okna (nakreslí okraj, prekážky, ovocie, hady).
 */
void draw_game_state(WINDOW *win, const GameState *gs);

#endif
