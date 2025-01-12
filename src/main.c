#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "server.h"
#include "client.h"

static int get_num_players() {
    int num;
    printf("Zadajte počet hráčov (1 alebo 2): ");
    scanf("%d", &num);
    if (num < 1 || num > 2) {
        printf("Neplatné.\n");
        exit(1);
    }
    return num;
}

static int get_game_mode() {
    int mode;
    printf("1. Hra bez prekážok (štandardný)\n");
    printf("2. Hra s prekážkami (prípadne časový limit)\n");
    printf("Zadajte (1 alebo 2): ");
    scanf("%d", &mode);
    if (mode < 1 || mode > 2) {
        printf("Neplatné.\n");
        exit(1);
    }
    return mode;
}

int main() {
    while (1) {
        printf("\n--- MENU ---\n");
        printf("1. Nová hra (spusti server a prípadne 1 klient)\n");
        printf("2. Pripojiť sa ku hre (klient)\n");
        printf("3. Koniec\n");
        printf("> ");
        int volba;
        scanf("%d", &volba);

        if (volba == 1) {
            int num_players = get_num_players();
            int game_mode = get_game_mode();
            int game_time = 0;
            if (game_mode == 2) {
                printf("Zadaj čas (sekundy, napr. 60): ");
                scanf("%d", &game_time);
            }

            // 1) SERVER
            pid_t spid = fork();
            if (spid == 0) {
                // dieťa => server
                Server server;
                init_server(&server, 8080, game_mode, game_time, num_players);
                start_server(&server);
                exit(0);
            }
            // počkáme sekundu, nech sa server rozbehne
            sleep(1);

            // 2) spustíme LEN 1 klienta, aj keby bola voľba 2 hráčov
            //    => ak user vybral 2 hráčov, druhý sa pripojí z iného terminálu 
            if (num_players >= 1) {
                pid_t cpid = fork();
                if (cpid == 0) {
                    // client
                    init_client("127.0.0.1", 8080);
                    start_client();
                    exit(0);
                }
            }

            // 3) počkáme, kým skončí server (a potom aj deti)
            wait(NULL); // server končí
            // počkáme na klienta
            wait(NULL);
        }
        else if (volba == 2) {
            // len klient (napr. 2. hráč)
            init_client("127.0.0.1", 8080);
            start_client();
        }
        else if (volba == 3) {
            printf("Koniec.\n");
            break;
        }
        else {
            printf("Neplatná voľba.\n");
        }
    }
    return 0;
}
