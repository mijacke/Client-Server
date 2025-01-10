#include "client.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ncurses.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <server|client>\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "server") == 0) {
        int game_mode, game_time = 0;

        printf("Zadajte režim hry (0 = štandardný, 1 = časový): ");
        scanf("%d", &game_mode);

        if (game_mode == 1) {
            printf("Zadajte čas hry (v sekundách): ");
            scanf("%d", &game_time);
        }

        Server server;
        init_server(&server, 8080, game_mode, game_time);
        start_game(&server);
        close_server(&server);
    } else if (strcmp(argv[1], "client") == 0) {
        init_client("127.0.0.1", 8080);
        start_client();
    } else {
        fprintf(stderr, "Neplatný argument. Použite 'server' alebo 'client'.\n");
        exit(1);
    }

    return 0;
}
