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
#include "server.h"

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
    int score = 0;

    int n = recv(sock, &server_time, sizeof(server_time), 0);
    if (n <= 0) {
        printf("Spojenie so serverom bolo ukončené.\n");
        goto end;
    }

    recv(sock, &start_time, sizeof(start_time), 0);
    printf("Prijatý čas hry: %d sekúnd, začiatok hry: %ld\n", server_time, start_time);

    // Prijatie počtu prekážok
    int num_obstacles;
    recv(sock, &num_obstacles, sizeof(num_obstacles), 0);

    // Ak nie sú prekážky, alokácia nebude potrebná
    Obstacle *obstacles = NULL;
    if (num_obstacles > 0) {
        obstacles = malloc(num_obstacles * sizeof(Obstacle));
        if (!obstacles) {
            perror("Chyba pri alokácii pamäte pre prekážky");
            exit(1);
        }
        recv(sock, obstacles, num_obstacles * sizeof(Obstacle), 0);
        printf("Prijaté prekážky:\n");
        for (int i = 0; i < num_obstacles; i++) {
            printf("Prekážka %d: (%d, %d)\n", i, obstacles[i].x, obstacles[i].y);
        }
    } else {
        printf("Žiadne prekážky.\n");
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
    int paused = 0; // Premenná na sledovanie pauzy

    init_snake(&snake, BOARD_WIDTH / 2, BOARD_HEIGHT / 2);
    int fruit_x = rand() % (BOARD_WIDTH - 2) + 1;
    int fruit_y = rand() % (BOARD_HEIGHT - 2) + 1;


    while (1) {
        if (!paused) {

            char signal;
            n = recv(sock, &signal, sizeof(signal), MSG_DONTWAIT);  // Non-blocking prijatie signálu
            if (n > 0 && signal == 'E') {
                mvprintw(BOARD_HEIGHT + 3, 0, "Koniec hry: Narazili ste!");
                refresh();
                break;  // Ukončite hru na základe signálu od servera
            }

            time_t current_time = time(NULL);
            int remaining_time = (server_time > 0) ? server_time - difftime(current_time, start_time) : INT64_MAX;

            if (server_time == 0) {
                mvprintw(BOARD_HEIGHT + 1, 0, "Standardny rezim: nekonecny cas.");
            } else if (remaining_time <= 0) {
                mvprintw(BOARD_HEIGHT + 1, 0, "Cas vyprsal! Koniec hry.");
                refresh();
                break;
            } else {
                mvprintw(BOARD_HEIGHT + 1, 0, "Zostavajuci cas: %d sekund", remaining_time);
            }

            mvprintw(BOARD_HEIGHT + 2, 0, "Skóre: %d", score);
            refresh();

            draw_board(win, snake.x[0], snake.y[0], fruit_x, fruit_y);
            draw_snake(win, &snake);

            // Vykreslenie prekážok
            if(num_obstacles > 0) {
                for (int i = 0; i < num_obstacles; i++) {
                    mvwaddch(win, obstacles[i].y, obstacles[i].x, '#');
                    mvprintw(BOARD_HEIGHT + 3 + i, 0, "Prekážka %d: (%d, %d)", i, obstacles[i].x, obstacles[i].y);  // Debug výpis
                }
            }

            move_snake(&snake, current_direction, grow);

            // Kontrola kolízie
            if (check_collision(&snake, NULL, 0, BOARD_WIDTH, BOARD_HEIGHT, &(Game){ .num_obstacles = num_obstacles, .obstacles = obstacles })) {
                mvprintw(BOARD_HEIGHT + 3, 0, "Koniec hry: Narazili ste!");
                refresh();
                break;  // Koniec hry
            }

            if (snake.x[0] == fruit_x && snake.y[0] == fruit_y) {
                grow = 1;
                score++;
                fruit_x = rand() % (BOARD_WIDTH - 2) + 1;
                fruit_y = rand() % (BOARD_HEIGHT - 2) + 1;
            } else {
                grow = 0;
            }

            wrefresh(win); // Aktualizácia okna po vykreslení všetkých prvkov
        } else {
            mvprintw(BOARD_HEIGHT + 2, 0, "Hra pozastavená. Stlačte 'p' na pokračovanie.");
            refresh();
        }

        int ch = wgetch(win);
        if (ch != ERR) {
            if (paused && ch != 'p') {
                continue; // Ak je pauza, ignorujeme všetky klávesy okrem 'p'
            }
            switch (ch) {
                case 'w':
                    if (!paused && current_direction != 's') current_direction = 'w';
                    break;
                case 's':
                    if (!paused && current_direction != 'w') current_direction = 's';
                    break;
                case 'a':
                    if (!paused && current_direction != 'd') current_direction = 'a';
                    break;
                case 'd':
                    if (!paused && current_direction != 'a') current_direction = 'd';
                    break;
                case 'p':
                    paused = !paused; // Prepnutie pauzy
                    break;
                case 'q':
                    goto end;
            }

            if (!paused) {
                if (send(sock, &ch, sizeof(ch), 0) <= 0) {
                    printf("Spojenie so serverom bolo ukončené.\n");
                    break;
                }
            }
        }

        usleep(200000); // Rýchlosť hry
    }

end:
    endwin();
}
