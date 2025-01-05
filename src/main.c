#include "client.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Použitie: %s <server|client>\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "server") == 0) {
        // Spustenie servera
        Server server;
        init_server(&server, 8080);  // Inicializuje server na porte 8080
        start_game(&server);  // Začne hru
        close_server(&server);  // Uzavrie server po skončení
    } 
    else if (strcmp(argv[1], "client") == 0) {
        // Spustenie klienta
        init_client("127.0.0.1", 8080);  // Pripojí sa na server s IP 127.0.0.1 a portom 8080
        start_client();  // Začne hru pre klienta
    } 
    else {
        fprintf(stderr, "Neplatný argument. Použite 'server' alebo 'client'.\n");
        exit(1);
    }

    return 0;
}

