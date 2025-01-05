#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ncurses.h>

#define BOARD_WIDTH 30
#define BOARD_HEIGHT 10

int sock;  // Socket pre komunikáciu s serverom

// Funkcia na inicializáciu klienta a pripojenie k serveru
void init_client(const char *server_address, int port) {
    struct sockaddr_in server_addr;

    // Vytvorenie socketu
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Nepodarilo sa vytvoriť socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Konverzia adresy na formát IP
    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0) {
        perror("Chyba pri konverzii adresy");
        exit(1);
    }

    // Pripojenie k serveru
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Nepodarilo sa pripojiť k serveru");
        exit(1);
    }
}

// Funkcia na spustenie klienta a vykonávanie herného cyklu
void start_client() {
    initscr();  // Inicializácia ncurses
    noecho();  // Zakáže zobrazenie zadaných znakov
    curs_set(FALSE); 
    WINDOW *win = newwin(BOARD_HEIGHT, BOARD_WIDTH, 0, 0);  // Vytvorí okno
    keypad(win, TRUE);  // Povolenie šípok

    int snake_x = BOARD_WIDTH / 2, snake_y = BOARD_HEIGHT / 2;  

    int fruit_x = rand() % (BOARD_WIDTH - 2) + 1, fruit_y = rand() % (BOARD_HEIGHT - 2) + 1;  
    
    while (1) {
        draw_board(win, snake_x, snake_y, fruit_x, fruit_y);  

        int ch = wgetch(win);  
        if (ch == 'q') break;  

        switch (ch) {
            case KEY_UP:
                snake_y--;
                break;
            case KEY_DOWN:  
                snake_y++;
                break;
            case KEY_LEFT:  
                snake_x--;
                break;
            case KEY_RIGHT:  
                snake_x++;
                break;
        }

        // Posielanie pohybu serveru
        char buffer[1] = {ch};  // iba 1 znak
        send(sock, buffer, sizeof(buffer), 0);

        // Zabezpečenie, že hadík ostáva v rámci herného poľa
        if (snake_x <= 0) snake_x = 1;
        if (snake_x >= BOARD_WIDTH - 1) snake_x = BOARD_WIDTH - 2;
        if (snake_y <= 0) snake_y = 1;
        if (snake_y >= BOARD_HEIGHT - 1) snake_y = BOARD_HEIGHT - 2;

        usleep(200000); 
    }

    close(sock);  
    endwin();  // Ukončenie ncurses
}

