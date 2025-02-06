#include "client.h"
#include "game.h"
#include "renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ncurses.h>
#include <pthread.h>

// ------------------------------------
// Globálne premené na strane klienta
// ------------------------------------
static int sock = -1;                      // Socket pre komunikáciu so serverom
static volatile int running = 1;           // 1 = klient beží, 0 = ukončuje sa

// Zdiľanie GmeState medzi vláknami
static GameState current_game_state;
static pthread_mutex_t game_state_mutex = PTHREAD_MUTEX_INITIALIZER;

static void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * Vlákno, ktoré nepretržite prijíma GameState zo servera a ukladá ho
 * do premennej current_game_state.
 */
static void* receiver_thread_func(void *arg) {
    (void)arg;

    while (running) {
        GameState st;
        int n = recv(sock, &st, sizeof(st), 0);
        if (n == 0) {
            // Server zavrel spojenie
            fprintf(stderr, "[Receiver] Server ukončil spojenie.\n");
            running = 0;
            break;
        } else if (n < 0) {
            // Chyba čítani
            usleep(50 * 1000);
            continue;
        } else if (n == sizeof(st)) {
            // Máme validný GameState
            pthread_mutex_lock(&game_state_mutex);
            current_game_state = st;
            pthread_mutex_unlock(&game_state_mutex);
        }
        // Trochu spomalenia, aby sa necyklilo príliš rýchlo
        usleep(50 * 1000);
    }
    return NULL;
}

void init_client(const char *server_address, int port) {
    struct sockaddr_in server_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        exit(1);
    }

    // Nastavíme socket do neblokujúceho režimu (je to uzitočne ale nie nutne)
    set_nonblocking(sock);

    printf("Klient pripojený k %s:%d\n", server_address, port);

    // Vynulovanie current_game_state
    memset(&current_game_state, 0, sizeof(current_game_state));
    current_game_state.game_over = 0;
    current_game_state.paused = 0;
}

void start_client() {
    // Spustenie ncurses
    initscr();
    noecho();
    curs_set(FALSE);

    WINDOW *win = newwin(BOARD_HEIGHT, BOARD_WIDTH, 0, 0);
    keypad(win, TRUE);
    nodelay(win, TRUE);

    pthread_t receiver_thread;
    if (pthread_create(&receiver_thread, NULL, receiver_thread_func, NULL) != 0) {
        perror("pthread_create");
        endwin();
        close(sock);
        exit(1);
    }

    // Hlavné vlákno: číta vstup od používateľa a posiela ho na server
    // + vykresľovanie
    while (running) {
        // 1) Prečítanie klávesu od užívateľa
        int ch = wgetch(win);
        if (ch != ERR) {
            // Poslanie znaku serveru
            char c = (char)ch;
            if (send(sock, &c, 1, 0) <= 0) {
                mvprintw(BOARD_HEIGHT + 1, 0, "Spojenie so serverom skončilo.");
                refresh();
                running = 0;
                break;
            }
            // Ak stlačil 'q', aj my končíme
            if (c == 'q') {
                running = 0;
                break;
            }
        }

        // 2) Vykreslenie aktuálneho stavu hry (ktorý možno upravil receiver_thread)
        pthread_mutex_lock(&game_state_mutex);
        GameState st_local = current_game_state; // lokálna kópia
        pthread_mutex_unlock(&game_state_mutex);

        werase(win);
        draw_game_state(win, &st_local);

        for (int p = 0; p < st_local.num_players; p++) {
            mvprintw(BOARD_HEIGHT + 1 + p, 0,
                     "Player %d | Score: %d", p, st_local.score[p]);
        }

        time_t now = time(NULL);
        double elapsed = difftime(now, st_local.start_time);
        mvprintw(BOARD_HEIGHT + 1 + st_local.num_players, 0,
                 "Game time: %.1f s", elapsed);

        // Ak je pauza, alebo odpočet
        if (st_local.paused) {
            mvprintw(BOARD_HEIGHT+2+st_local.num_players, 0, "[PAUSED]");
            if (st_local.countdown > 0) {
                mvprintw(BOARD_HEIGHT+3+st_local.num_players, 0,
                         "Resume in %d...", st_local.countdown);
            }
        }

        // Ak server signalizuje game_over, ukončime aj my
        if (st_local.game_over) {
            mvprintw(BOARD_HEIGHT + 3 + st_local.num_players,
                     0, "Koniec hry. Stlačte q.");
            refresh();
            // Počkáme, kým užívateľ stlačí q
            int tmp;
            while ((tmp = wgetch(win)) != 'q' && tmp != ERR) {
            }
            running = 0;
        }

        refresh();

        usleep(100 * 1000);
    }

    pthread_join(receiver_thread, NULL);

    endwin();

    if (sock >= 0) {
        close(sock);
        sock = -1;
    }

    printf("Klient ukončený.\n");
}
