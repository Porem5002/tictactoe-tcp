#include <stdio.h>

#include "server.h"

server_t server_make(SOCKET sock)
{
    server_t server = {0};
    server.sock = sock;
    return server;
}

void server_wait_for_action(server_t* server)
{
    FD_ZERO(&server->fds);
    FD_SET(server->sock, &server->fds);

    for(size_t i = 0; i < server->room_count; i++)
        room_fd_set(&server->rooms[i], &server->fds);

    int n_select = select(0, &server->fds, NULL, NULL, NULL);
    if(n_select == -1)
    {
        fprintf(stderr, "ERROR: Select failed %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
}

void server_accept_client(server_t* server)
{
    if(!FD_ISSET(server->sock, &server->fds)) return;

    SOCKET cli_sock = accept(server->sock, NULL, NULL);
    if(cli_sock == INVALID_SOCKET) return;
    
    connection_t cli_conn = connection_init(cli_sock, false);

    if(server->room_count > 0 && server->rooms[server->room_count - 1].mode == ROOM_MODE_SETUP)
    {
        room_t* curr = &server->rooms[server->room_count - 1];
        room_connect_second(curr, cli_conn);
    }
    else
    {
        room_t* curr = &server->rooms[server->room_count];
        *curr = room_make(cli_conn);
        server->room_count++;
    }
}

void server_update_rooms(server_t* server)
{
    for(size_t i = 0; i < server->room_count;)
    {
        room_t* curr = &server->rooms[i];

        if(!room_update(curr, &server->fds))
        {
            room_free(&server->rooms[i]);
            memmove(curr, curr + 1, (server->room_count - i - 1) * sizeof(room_t));
            server->room_count--;
            continue;
        }

        i++;
    }
}

void server_free(server_t* server)
{
    for(size_t i = 0; i < server->room_count; i++)
        room_free(&server->rooms[i]);

    closesocket(server->sock);
}