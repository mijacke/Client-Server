#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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

    if (game_mode < 1 || game_mode > 2) {
        printf("Neplatný výber. Zadajte 1 alebo 2.\n");
        exit(1);
    }

    return game_mode;
}

int main() {
    while (1) {
        printf("\nHlavné menu:\n");
        printf("1. Nová hra\n");
        printf("2. Pripojenie k hre\n");
        printf("3. Koniec\n");
        printf("Zadajte možnosť: ");

        int choice;
        scanf("%d", &choice);

        if (choice == 1) {
            int num_players = get_num_players(); // Získanie počtu hráčov
            int game_mode = get_game_mode();     // Získanie režimu hry
            int game_time = 0;
            int has_obstacles = (game_mode == 2) ? 1 : 0; // Ak je režim 2, nastavíme prekážky

            if (game_mode == 2) {
                printf("Zadajte čas hry (v sekundách): ");
                scanf("%d", &game_time);
            }

            pid_t pid1 = fork();
            if (pid1 < 0) {
                perror("Chyba pri vytváraní serverového procesu");
                exit(1);
            } else if (pid1 == 0) {
                Server server;
                init_server(&server, 8080, game_mode, game_time, num_players);
                start_game(&server, has_obstacles);
                close_server(&server);
                exit(0);
            }

            sleep(2);

            for (int i = 0; i < num_players; i++) {
                pid_t pid2 = fork();
                if (pid2 < 0) {
                    perror("Chyba pri vytváraní klientského procesu");
                    exit(1);
                } else if (pid2 == 0) {
                    init_client("127.0.0.1", 8080);
                    start_client();
                    exit(0);
                }
            }

            wait(NULL); // Čakáme na ukončenie servera
            for (int i = 0; i < num_players; i++) {
                wait(NULL);
            }
        } else if (choice == 2) {
            init_client("127.0.0.1", 8080);
            start_client();
        } else if (choice == 3) {
            printf("Aplikácia sa ukončuje.\n");
            break;
        } else {
            printf("Neplatná možnosť, skúste znova.\n");
        }
    }
    return 0;
}
