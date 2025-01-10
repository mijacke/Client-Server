#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "client.h"

// Funkcia na získanie počtu hráčov
int get_num_players() {
    int num_players;
    printf("Vyberte počet hráčov:\n");
    printf("1. Pre 1 hráča\n");
    printf("2. Pre 2 hráčov\n");
    printf("Zadajte číslo počtu hráčov (1 alebo 2): ");
    scanf("%d", &num_players);
    if (num_players < 1 || num_players > 2) {
        printf("Počet hráčov musí byť 1 alebo 2.\n");
        exit(1);
    }
    return num_players;
}

// Funkcia na získanie režimu hry (s prekážkami alebo bez)
int get_game_mode() {
    int game_mode;
    printf("Vyberte herný režim:\n");
    printf("1. Hra bez prekážok\n");
    printf("2. Hra s prekážkami\n");
    printf("Zadajte číslo režimu (1 alebo 2): ");
    scanf("%d", &game_mode);

    return game_mode;  // 1 pre hru bez prekážok, 2 pre hru s prekážkami
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <server|client>\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "server") == 0) {
        // Výber počtu hráčov
        int num_players = get_num_players();

        // Výber režimu hry
        int game_mode = get_game_mode();

        int game_time = 0;

        if (game_mode == 2) {
            // Zadajte čas hry len v časovom režime
            printf("Zadajte čas hry (v sekundách): ");
            scanf("%d", &game_time);
        }

        Server server;
        init_server(&server, 8080, game_mode, game_time, num_players);  // Inicializácia servera
        start_game(&server);  // Tu sa začne hra podľa režimu
        close_server(&server);
    } else if (strcmp(argv[1], "client") == 0) {
        // Pre klienta môžeme nastaviť serverovú adresu a port
        init_client("127.0.0.1", 8080);
        start_client();
    } else {
        fprintf(stderr, "Neplatný argument. Použite 'server' alebo 'client'.\n");
        exit(1);
    }

    return 0;
}
