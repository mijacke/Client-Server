#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include "game.h"

static void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void broadcast_game_state(Server *server, const GameState *st) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->players[i].active) {
            send(server->players[i].client_socket, st, sizeof(*st), 0);
        }
    }
}

/**
 * Naplní GameState na základe server->players, server->fruit_x[], ...
 */
static void fill_game_state(Server *server, GameState *st) {
    memset(st, 0, sizeof(GameState));

    // Koľko hráčov je aktívnych?
    int count = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->players[i].active) {
            count++;
        }
    }
    st->num_players = count;

    // Prejdeme hráčov, uložíme do st
    int idx = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!server->players[i].active) continue;
        // snake
        for (int seg = 0; seg < server->players[i].snake.length; seg++) {
            st->snake_x[idx][seg] = server->players[i].snake.x[seg];
            st->snake_y[idx][seg] = server->players[i].snake.y[seg];
        }
        st->snake_length[idx] = server->players[i].snake.length;
        st->score[idx] = server->players[i].score;
        idx++;
    }

    // Počet ovocia = toľko, koľko je hráčov (count)
    st->fruit_count = count;
    for (int i = 0; i < count; i++) {
        st->fruit_x[i] = server->fruit_x[i];
        st->fruit_y[i] = server->fruit_y[i];
    }

    // Prekážky
    st->num_obstacles = server->num_obstacles;
    for (int i = 0; i < server->num_obstacles; i++) {
        st->obstacles[i] = server->obstacles[i];
    }

    st->start_time = server->start_time;

    st->game_over = 0;

    st->paused = server->paused;
    st->countdown = 0; // default, pri bežnom stave
}

/**
 * Pošle countdown 3..2..1 všetkým hráčom
 *  - každú sekundu upraví st->countdown a spraví broadcast
 */
static void resume_countdown(Server *server) {
    for (int c = 3; c > 0; c--) {
        GameState st;
        fill_game_state(server, &st);
        st.countdown = c;         // tu nastavujeme odpočet
        broadcast_game_state(server, &st);
        sleep(1);
    }
    // Po skončení countDown už st.countdown=0 => bežná hra
}

void init_server(Server *server, int port, int game_mode, int game_time, int max_players) {
    server->game_mode = game_mode;
    server->game_time = game_time;
    server->start_time = time(NULL);
    server->active_players = 0;
    server->running = 1;
    server->max_players = max_players;

    server->paused = 0;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        server->players[i].active = 0;
        server->players[i].client_socket = -1;
        server->players[i].score = 0;
        server->fruit_x[i] = -1;
        server->fruit_y[i] = -1;
    }

    server->num_obstacles = 0;

    if (game_mode == 2) {
        load_obstacles_from_file(server->obstacles, &server->num_obstacles, "../src/obstacles.txt");
    }

    srand(time(NULL));
    for (int i = 0; i < MAX_PLAYERS; i++) {
        server->fruit_x[i] = rand() % (BOARD_WIDTH - 2) + 1;
        server->fruit_y[i] = rand() % (BOARD_HEIGHT - 2) + 1;
    }

    // Socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        exit(1);
    }

    if (listen(sock, 5) < 0) {
        perror("listen");
        close(sock);
        exit(1);
    }

    printf("Server beží na porte %d\n", port);
    server->socket = sock;
    set_nonblocking(server->socket);
}

/**
 * Pomocná funkcia, ktorá priradí nového klienta
 */
static int add_new_client(Server *server, int client_fd) {
    // Ak by pripojením tohto klienta prekročilo povolený max_players:
    if (server->active_players >= server->max_players) {
        // Odmietnuť
        return -1;
    }
    // Inak hľadaj voľný slot
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!server->players[i].active) {
            server->players[i].client_socket = client_fd;
            server->players[i].active = 1;

            init_snake(&server->players[i].snake, BOARD_WIDTH / 2, BOARD_HEIGHT / 2 + i);

            server->players[i].score = 0;
            server->active_players++;
            return i;
        }
    }
    return -1;
}


void start_server(Server *server) {
    time_t last_activity = time(NULL); 

    while (server->running) {
        // 1) accept
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int cfd = accept(server->socket, (struct sockaddr *)&caddr, &clen);
        if (cfd >= 0) {
            set_nonblocking(cfd);
            int idx = add_new_client(server, cfd);
            if (idx < 0) {
                close(cfd);
                printf("Server je plný, odmietam klienta.\n");
            } else {
                printf("Nový klient na index %d\n", idx);
            }
        }

        // 2) čítanie kláves
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!server->players[i].active) continue;
            int sock = server->players[i].client_socket;
            char buf[1];
            int n = recv(sock, buf, 1, 0);
            if (n == 0) {
                // odpojenie
                printf("Hráč %d sa odpojil.\n", i);
                close(sock);
                server->players[i].active = 0;
                server->active_players--;
            } else if (n > 0) {
                char c = buf[0];
                switch (c) {
                    case 'w':
                    case 'a':
                    case 's':
                    case 'd':
                        if (!server->paused) {
                            server->players[i].snake.last_direction = c;
                        }
                        break;
                    case 'p':
                        if (!server->paused) {
                            // ideme zapnúť pauzu
                            server->paused = 1;
                        } else {
                            // pauza bola, tak reštart hry => countdown
                            resume_countdown(server);
                            server->paused = 0;
                        }
                        break;
                    case 'q':
                        close(sock);
                        server->players[i].active = 0;
                        server->active_players--;
                        break;
                    default:
                        // iné klávesy ignorujeme
                        break;
                }
            }
        }

        if (!server->paused) {
            // 3) pohyb + kolízie
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (!server->players[i].active) continue;
                move_snake(&server->players[i].snake, BOARD_WIDTH, BOARD_HEIGHT);

                // kolízia
                int col = check_collision(&server->players[i].snake,
                                        server->players,
                                        MAX_PLAYERS,
                                        BOARD_WIDTH,
                                        BOARD_HEIGHT,
                                        server->obstacles,
                                        server->num_obstacles);
                if (col) {
                    printf("Hráč %d narazil! Koniec pre neho.\n", i);
                    close(server->players[i].client_socket);
                    server->players[i].active = 0;
                    server->active_players--;
                    continue;
                }

                // +++ TERAZ kontrola ovocia. 
                //  Ak je v hre X hráčov, je X ovocí
                //  => server->active_players = count => je to toľko reálne používaných ovocí
                //  => index 0..(active_players-1)
                Snake *sn = &server->players[i].snake;
                for (int f = 0; f < server->active_players; f++) {
                    if (sn->x[0] == server->fruit_x[f] &&
                        sn->y[0] == server->fruit_y[f]) {
                        server->players[i].score++;
                        grow_snake(sn);
                        // Vygeneruj nové ovocie pre index f
                        server->fruit_x[f] = rand() % (BOARD_WIDTH - 2) + 1;
                        server->fruit_y[f] = rand() % (BOARD_HEIGHT - 2) + 1;
                    }
                }
            }
        }

        // 4) Pošleme GameState
        GameState st;
        fill_game_state(server, &st);

        // Ak je časový režim a vypršal
        if (server->game_mode == 2) {
            time_t now = time(NULL);
            if (difftime(now, server->start_time) >= server->game_time) {
                printf("Čas vypršal, koniec hry.\n");
                st.game_over = 1; // nastaviť game_over
                broadcast_game_state(server, &st);
                break;
            }
        } else {
            // Ak uplynulo 10 s a active_players=0 => mozes skoncit
            if (server->active_players == 0) {
                time_t now = time(NULL);
                if (difftime(now, last_activity) >= 10.0) {
                    printf("10 sekúnd bez hráčov, končím.\n");
                    st.game_over = 1;
                    broadcast_game_state(server, &st);
                    break;
                }
            }
        }

        st.paused = server->paused;
        broadcast_game_state(server, &st);

        usleep(200000);
    }
    stop_server(server);
}

void stop_server(Server *server) {
    server->running = 0;
    if (server->socket >= 0) {
        close(server->socket);
        server->socket = -1;
    }
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (server->players[i].active) {
            close(server->players[i].client_socket);
            server->players[i].active = 0;
        }
    }
    printf("Server ukončený.\n");
}

/**
 * Kontrola kolízií hadíka
 */
int check_collision(const Snake *snake,
                    const Player *players, int num_players,
                    int width, int height,
                    const Obstacle *obstacles, int num_obstacles)
{
    // so sebou
    for (int i = 1; i < snake->length; i++) {
        if (snake->x[0] == snake->x[i] &&
            snake->y[0] == snake->y[i]) {
            return 1;
        }
    }
    // iní hráči
    for (int p = 0; p < num_players; p++) {
        if (!players[p].active) continue;
        const Snake *other = &players[p].snake;
        if (other == snake) continue;
        for (int i = 0; i < other->length; i++) {
            if (snake->x[0] == other->x[i] &&
                snake->y[0] == other->y[i]) {
                return 1;
            }
        }
    }
    // prekážky
    for (int i = 0; i < num_obstacles; i++) {
        if (snake->x[0] == obstacles[i].x &&
            snake->y[0] == obstacles[i].y) {
            return 1;
        }
    }

    return 0;
}
