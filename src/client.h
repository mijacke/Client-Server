#ifndef CLIENT_H
#define CLIENT_H

#include <ncurses.h>

void init_client(const char *server_address, int port);
void start_client();

#endif
