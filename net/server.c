#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <winsock2.h>

#include "..\src\base.h"
#include "net.h"

typedef struct
{
    bool full;
    connection_t conns [2];
    game_t game;
} instance_t;

void instance_free(instance_t* instance)
{
    connection_close(&instance->conns[0]);

    if(instance->full)
    {
        connection_close(&instance->conns[1]);
        game_free(&instance->game);
    }
}

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "ERROR: Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    char* endp;
    long port = strtol(argv[1], &endp, 10);
    if(port < 0 || *endp != '\0')
    {
        fprintf(stderr, "ERROR: Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    net_start();

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET)
    {
        fprintf(stderr, "ERROR: Could not create socket!\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons((u_short)port);

    if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: Could not bind socket!\n");
        exit(EXIT_FAILURE);
    }

    if(listen(sock, 10) == SOCKET_ERROR)
    {
        fprintf(stderr, "ERROR: Could not start listenning!\n");
        exit(EXIT_FAILURE);
    }

    size_t instance_count = 0;
    instance_t instances [10] = {0};

    fd_set fds;

    while(true)
    {
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        for(size_t i = 0; i < instance_count; i++)
        {
            FD_SET(instances[i].conns[0].sock, &fds);

            if(instances[i].full)
                FD_SET(instances[i].conns[1].sock, &fds);
        }

        int n_select = select(0, &fds, NULL, NULL, NULL);
        if(n_select == -1)
        {
            fprintf(stderr, "ERROR: Select failed %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        if(FD_ISSET(sock, &fds))
        {
            SOCKET cli_sock = accept(sock, NULL, NULL);
            if(cli_sock != INVALID_SOCKET)
            {
                connection_t cli_conn = connection_init(cli_sock, false);

                if(instance_count > 0 && !instances[instance_count - 1].full)
                {
                    instance_t* curr = &instances[instance_count - 1];

                    curr->full = true;
                    curr->conns[1] = cli_conn;
                    curr->game = game_make(3, PLAYER_1);
                
                    packet_t p = {0};
                    p.kind = PACKET_KIND_RESPONSE_CONNECT;
                    p.response_connect.assigned_player = PLAYER_1;
                    packet_send(&curr->conns[0], p);

                    p.kind = PACKET_KIND_RESPONSE_CONNECT;
                    p.response_connect.assigned_player = PLAYER_2;
                    packet_send(&curr->conns[1], p);
                }
                else
                {
                    instance_t* curr = &instances[instance_count];
                    curr->full = false;
                    curr->conns[0] = cli_conn;
                    instance_count++;
                }
            }
        }

        for(size_t i = 0; i < instance_count;)
        {
            if(!instances[i].full)
            {
                packet_t p;
                
                if(FD_ISSET(instances[i].conns[0].sock, &fds) && packet_poll(&instances[i].conns[0], &p) == PACKET_STATUS_NO_MORE)
                {
                    instance_free(&instances[i]);
                    memmove(instances + i, instances + i + 1, (instance_count - i - 1) * sizeof(instance_t));
                    instance_count--;
                    continue;
                }

                i++;
                continue;
            }

            if(FD_ISSET(instances[i].conns[0].sock, &fds))
            {
                packet_t p;
                packet_status_t status = packet_poll(&instances[i].conns[0], &p);
            
                if(status == PACKET_STATUS_RECEIVED)
                {
                    int x = p.resquest_move.x;
                    int y = p.resquest_move.y;

                    if(instances[i].game.curr_player == PLAYER_1 && game_do_move(&instances[i].game, x, y))
                    {
                        packet_t p = {0};
                        p.kind = PACKET_KIND_RESPONSE_MOVE;
                        p.response_move.player = PLAYER_1;
                        p.response_move.x = x;
                        p.response_move.y = y;

                        packet_send(&instances[i].conns[0], p);
                        packet_send(&instances[i].conns[1], p);
                    }
                }
                
                if(status == PACKET_STATUS_NO_MORE || !instances[i].game.running)
                {
                    instance_free(&instances[i]);
                    memmove(instances + i, instances + i + 1, (instance_count - i - 1) * sizeof(instance_t));
                    instance_count--;
                    continue;
                }
            }

            if(FD_ISSET(instances[i].conns[1].sock, &fds))
            {
                packet_t p;
                packet_status_t status = packet_poll(&instances[i].conns[1], &p);
            
                if(status == PACKET_STATUS_RECEIVED)
                {
                    int x = p.resquest_move.x;
                    int y = p.resquest_move.y;

                    if(instances[i].game.curr_player == PLAYER_2 && game_do_move(&instances[i].game, x, y))
                    {
                        packet_t p = {0};
                        p.kind = PACKET_KIND_RESPONSE_MOVE;
                        p.response_move.player = PLAYER_2;
                        p.response_move.x = x;
                        p.response_move.y = y;

                        packet_send(&instances[i].conns[0], p);
                        packet_send(&instances[i].conns[1], p);
                    }
                }

                if(status == PACKET_STATUS_NO_MORE || !instances[i].game.running)
                {
                    instance_free(&instances[i]);
                    memmove(instances + i, instances + i + 1, (instance_count - i - 1) * sizeof(instance_t));
                    instance_count--;
                    continue;
                }
            }

            i++;
        }
    }

    for(size_t i = 0; i < instance_count; i++)
        instance_free(&instances[i]);

    closesocket(sock);
    
    net_finish();
    return 0;
}