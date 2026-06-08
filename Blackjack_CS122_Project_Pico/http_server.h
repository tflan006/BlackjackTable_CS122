#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "Player.h"

extern Player players[MAX_PLAYERS];

// Start the HTTP server on port 80
bool http_server_init();
void http_server_push_state(Player *player);

#endif