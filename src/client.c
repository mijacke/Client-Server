#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include <fcntl.h>
#include <time.h>
#include "game.h"
#include "renderer.h"

// Globálna premená pre socket
static int sock = -1;

// Nastaví socket do non-blocking módu
static void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
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

    // Nastavíme non-blocking
    set_nonblocking(sock);

    printf("Klient pripojený k %s:%d\n", server_address, port);
}

void start_client() {
    // ncurses init
    initscr();
    noecho();
    curs_set(FALSE);

    // Vytvoríme okno pre herné pole
    WINDOW *win = newwin(BOARD_HEIGHT, BOARD_WIDTH, 0, 0);
    keypad(win, TRUE);
    nodelay(win, TRUE);

    while (1) {
        // 1) Prečítanie klávesu od užívateľa
        int ch = wgetch(win);
        if (ch != ERR) {
            // Poslanie tohto znaku serveru
            if (send(sock, &ch, 1, 0) <= 0) {
                // Ak posielanie zlyhá, server ukončil spojenie
                mvprintw(BOARD_HEIGHT + 1, 0, "Spojenie so serverom skončilo.");
                refresh();
                break;
            }
            if (ch == 'q') {
                break;
            }
        }

        // 2) Pokus prečítať GameState od servera
        GameState st;
        int n = recv(sock, &st, sizeof(st), 0);
        if (n == 0) {
            // Server zavrel spojenie
            mvprintw(BOARD_HEIGHT + 2, 0, "Server ukončil spojenie.");
            refresh();
            break;
        } else if (n == sizeof(st)) {
            // Máme nový stav hry
            if (st.game_over == 1) {
                mvprintw(BOARD_HEIGHT + 2, 0, "Koniec hry.");
                refresh();
                break;
            }
           
            draw_game_state(win, &st);

            for (int p = 0; p < st.num_players; p++) {
                mvprintw(BOARD_HEIGHT + 1 + p, 0,
                         "Hráč %d | Skóre: %d", p, st.score[p]);
            }

            time_t now = time(NULL);
            double elapsed = difftime(now, st.start_time);
            mvprintw(BOARD_HEIGHT + 1 + st.num_players,
                     0, "Cas hry: %.1f s", elapsed);

            if (st.paused) {
                mvprintw(BOARD_HEIGHT+2+st.num_players, 0, "[PAUSED]");
                if (st.countdown > 0) {
                    // st.countdown = 3..2..1
                    mvprintw(BOARD_HEIGHT+3+st.num_players, 0,
                             "Resume in %d...", st.countdown);
                }
            }
            refresh();
        }

        usleep(100000);
    }

    endwin();
    if (sock >= 0) {
        close(sock);
        sock = -1;
    }
    printf("Klient ukončený.\n");
}
