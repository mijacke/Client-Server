#ifndef CLIENT_H
#define CLIENT_H

/**
 * Inicializuje klientsky socket a pripája sa na server.
 */
void init_client(const char *server_address, int port);

/**
 * Spustí hlavnú slučku klienta (ncurses, vykresľovanie).
 */
void start_client();

#endif
