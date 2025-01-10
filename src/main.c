#include "server.h"  // Zahrnutie správneho hlavičkového súboru pre Server
#include "client.h"  // Ak sú potrebné funkcie clienta
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Pridané na použitie strcmp

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <server|client>\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "server") == 0) {
        int game_mode, game_time = 0, num_players;

        printf("Zadajte režim hry (0 = štandardný, 1 = časový): ");
        scanf("%d", &game_mode);

        if (game_mode == 1) {
            printf("Zadajte čas hry (v sekundách): ");
            scanf("%d", &game_time);
        }

        printf("Zadajte počet hráčov (maximálne 4): ");
        scanf("%d", &num_players);

        if (num_players < 1 || num_players > MAX_PLAYERS) {
            printf("Počet hráčov musí byť medzi 1 a %d.\n", MAX_PLAYERS);
            exit(1);
        }

        Server server;
        init_server(&server, 8080, game_mode, game_time, num_players);
        start_game(&server);
        close_server(&server);
    } else if (strcmp(argv[1], "client") == 0) {
        init_client("127.0.0.1", 8080);  // Zavoláme funkciu na inicializáciu klienta
        start_client();  // A začneme hru
    } else {
        fprintf(stderr, "Neplatný argument. Použite 'server' alebo 'client'.\n");
        exit(1);
    }

    return 0;
}
