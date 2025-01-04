#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "game.h"

// Počet klientov (hráčov)
#define MAX_CLIENTS 2

pthread_mutex_t game_lock;

typedef struct {
    int sockfd;
    struct sockaddr_in address;
    int player_id;
} client_t;

// Funkcia na spracovanie vstupu pre každého klienta (hráča)
void* handle_client(void* arg) {
    client_t* client = (client_t*)arg;
    GameState state;
    
    // Inicializácia hry pre klienta
    initialize_game(&state);

    // Hra beží, kým klient nezastaví
    while (state.is_running) {
        // Môžeme pridať synchronizáciu, aby sa viacerí klienti neprekryli
        pthread_mutex_lock(&game_lock);
        update_game(&state);  // Aktualizuj stav hry
        pthread_mutex_unlock(&game_lock);

        // Tu môžeme implementovať prijímanie príkazov (napr. pohyb hadíka)
        usleep(200000);  // Spomalenie hry, aby sa nezrýchlil pohyb hadíka
    }

    close(client->sockfd);
    free(client);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);
    pthread_t threads[MAX_CLIENTS];

    // Inicializácia mutexu pre synchronizáciu
    pthread_mutex_init(&game_lock, NULL);

    // Vytvorenie serverového socketu
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080); // Port pre server

    // Bindovanie socketu
    if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Počúvanie na pripojenie
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port 8080\n");

    // Akceptovanie pripojení od klientov
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        client_t* client = (client_t*)malloc(sizeof(client_t));
        client->sockfd = client_fd;
        client->address = client_address;
        client->player_id = i;

        // Vytvorenie nového vlákna pre každého klienta
        pthread_create(&threads[i], NULL, handle_client, (void*)client);
    }

    // Počkajte, kým sa všetky vlákna dokončia
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Zatvorenie socketu
    close(server_fd);
    pthread_mutex_destroy(&game_lock);  // Zničenie mutexu

    return 0;
}

