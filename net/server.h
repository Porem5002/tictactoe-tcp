#ifndef SERVER_H
#define SERVER_H

#include "room.h"

typedef struct
{
    size_t room_count;
    room_t rooms [10]; // TODO: Maybe make room array dynamically allocated
    SOCKET sock;
    fd_set fds;
} server_t;

server_t server_make(SOCKET sock);
void server_wait_for_action(server_t* server);
void server_accept_client(server_t* server);
void server_update_rooms(server_t* server);
void server_free(server_t* server);

#endif