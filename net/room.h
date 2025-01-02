#ifndef ROOM_H
#define ROOM_H

#include <stdbool.h>

#include "net.h"

typedef enum
{
    ROOM_MODE_SETUP,
    ROOM_MODE_PLAYING,
    ROOM_MODE_FINISHED,
} room_mode_t;

typedef struct
{
    room_mode_t mode;
    connection_t conns [2];
    game_t game;
} room_t;

room_t room_make(connection_t cli_conn);
void room_connect_second(room_t* room, connection_t cli_conn);
bool room_update(room_t* room, fd_set* fds);
void room_fd_set(room_t* room, fd_set* fds);
void room_free(room_t* room);

#endif