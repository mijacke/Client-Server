#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>
#include "renderer.h"
#include "snake.h"
#include "game.h"

#define BOARD_WIDTH 30
#define BOARD_HEIGHT 10

int sock;  // Socket pre komunikáciu s serverom
int server_time;      // Celkový čas hry (len pre časový režim)
time_t start_time;    // Čas začiatku hry

// Funkcia na inicializáciu klienta a pripojenie k serveru
void init_client(const char *server_address, int port) {
    struct sockaddr_in server_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Nepodarilo sa vytvoriť socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Chyba pri konverzii adresy");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Nepodarilo sa pripojiť k serveru");
        exit(1);
    }
}

// Funkcia na spustenie klienta a vykonávanie herného cyklu
void start_client() {
    recv(sock, &server_time, sizeof(server_time), 0);
    recv(sock, &start_time, sizeof(start_time), 0);
    printf("Prijatý čas hry: %d sekúnd, začiatok hry: %ld\n", server_time, start_time);

    if (server_time == 0) {
        printf("Hra je v štandardnom režime. Čas sa ignoruje.\n");
    }

    initscr();
    noecho();
    curs_set(FALSE);
    WINDOW *win = newwin(BOARD_HEIGHT, BOARD_WIDTH, 0, 0);
    keypad(win, TRUE);
    nodelay(win, TRUE);

    Snake snake;
    int grow = 0;
    char current_direction = 'd';

    init_snake(&snake, BOARD_WIDTH / 2, BOARD_HEIGHT / 2);
    int fruit_x = rand() % (BOARD_WIDTH - 2) + 1;
    int fruit_y = rand() % (BOARD_HEIGHT - 2) + 1;

    while (1) {
        time_t current_time = time(NULL);
        int remaining_time = (server_time > 0) ? server_time - difftime(current_time, start_time) : INT64_MAX;

        if (remaining_time <= 0 && server_time > 0) {
            mvprintw(BOARD_HEIGHT + 1, 0, "Čas vypršal! Koniec hry.");
            refresh();
            break;
        }

        // Vykreslenie všetkých hadíkov a ovocia
        draw_board(win, snake.x[0], snake.y[0], fruit_x, fruit_y);
        draw_snake(win, &snake);
        mvprintw(BOARD_HEIGHT + 1, 0, "Zostávajúci čas: %d sekúnd", remaining_time);
        refresh();

        int ch = wgetch(win);
        if (ch != ERR) {
            switch (ch) {
                case 'w':
                    if (current_direction != 's') current_direction = 'w';
                    break;
                case 's':
                    if (current_direction != 'w') current_direction = 's';
                    break;
                case 'a':
                    if (current_direction != 'd') current_direction = 'a';
                    break;
                case 'd':
                    if (current_direction != 'a') current_direction = 'd';
                    break;
                case 'q':
                    goto end;
            }

            // Posielanie príkazov serveru
            send(sock, &ch, sizeof(ch), 0);
        }

        move_snake(&snake, current_direction, grow);

        if (snake.x[0] == fruit_x && snake.y[0] == fruit_y) {
            grow = 1;
            fruit_x = rand() % (BOARD_WIDTH - 2) + 1;
            fruit_y = rand() % (BOARD_HEIGHT - 2) + 1;
        } else {
            grow = 0;
        }

        usleep(200000);  // Pauza
    }

    end:
    endwin();
}
