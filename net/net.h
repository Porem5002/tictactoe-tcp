#ifndef PACKETS_H
#define PACKETS_H

#include <winsock2.h>

#include "..\base.h"

#define COMPACT_PACKET_SIZE 4

typedef enum
{
    CONNECT_STATUS_WAITING,
    CONNECT_STATUS_SUCCESS,
    CONNECT_STATUS_FAILED,
} connect_status_t;

typedef enum
{
    PACKET_STATUS_WAITING,
    PACKET_STATUS_RECEIVED,
    PACKET_STATUS_NO_MORE,
} packet_status_t;

typedef enum
{
    PACKET_KIND_RESPONSE_CONNECT,
    PACKET_KIND_RESPONSE_MOVE,

    PACKET_KIND_REQUEST_MOVE,
} packet_kind_t;

typedef struct
{
    packet_kind_t kind;

    union
    {
        struct 
        {
            player_t assigned_player;
        } response_connect;

        struct
        {
            player_t player;
            int x, y;
        } response_move;

        struct
        {
            int x, y;
        } resquest_move;
    };
} packet_t;

typedef struct
{
    SOCKET sock;
    size_t bytes_read;
    char data [COMPACT_PACKET_SIZE];
} connection_t;

connection_t connection_init(SOCKET sock, bool nonblock);
connect_status_t connection_start_connect(connection_t* conn, struct sockaddr_in addr);
connect_status_t connection_continue_connect(connection_t* conn);
void connection_close(connection_t* conn);

packet_status_t packet_poll(connection_t* connection, packet_t* out_packet);
bool packet_send(connection_t* connection, packet_t packet);

#endif