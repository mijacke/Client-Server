#ifndef GAME_H
#define GAME_H

#include "snake.h"
#include <time.h>

#define MAX_PLAYERS 2
#define MAX_OBSTACLES 5

/**
 * Reprezentácia jednej prekážky
 */
typedef struct {
    int x;
    int y;
} Obstacle;

/**
 * Toto je štruktúra, ktorá sa posiela z servera klientom
 * a obsahuje stav hry (čo treba na vykreslenie).
 */
typedef struct {
    int num_players; // koľko hráčov je reálne aktívnych

    // Pre každého hráča
    int snake_x[MAX_PLAYERS][MAX_SNAKE_LENGTH];
    int snake_y[MAX_PLAYERS][MAX_SNAKE_LENGTH];
    int snake_length[MAX_PLAYERS];
    int score[MAX_PLAYERS];

    // kazdy novy hadik = ovocie++
    int fruit_count;
    int fruit_x[MAX_PLAYERS];
    int fruit_y[MAX_PLAYERS];

    int num_obstacles;
    Obstacle obstacles[MAX_OBSTACLES];

    int game_over; // (0 = beží, 1 = koniec)
    time_t start_time;
} GameState;

/**
 * Načíta prekážky zo súboru do poľa obstacles
 */
void load_obstacles_from_file(Obstacle *obstacles, int *num_obstacles, const char *filename);

#endif
