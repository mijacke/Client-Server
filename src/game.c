#include "game.h"
#include <stdio.h>
#include <stdlib.h>

void load_obstacles_from_file(Obstacle *obstacles, int *num_obstacles, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Nepodarilo sa otvoriť súbor s prekážkami");
        return;
    }

    int count = 0;
    while (fscanf(file, "%d %d", &obstacles[count].x, &obstacles[count].y) == 2) {
        count++;
        if (count >= MAX_OBSTACLES) {
            break; // Narazili sme na maximum
        }
    }
    fclose(file);

    *num_obstacles = count;
    printf("Načítaných %d prekážok zo súboru '%s'.\n", count, filename);
}
