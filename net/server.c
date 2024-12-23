#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <winsock2.h>

#include "..\src\base.h"
#include "net.h"

int main()
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != INVALID_SOCKET);

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(6000);

    assert(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != SOCKET_ERROR);

    assert(listen(sock, 10) != SOCKET_ERROR);
    
    player_t players [] = { PLAYER_1, PLAYER_2 };
    connection_t connections [2] = {0};

    for(int i = 0; i < 2; i++)
    {
        SOCKET cli_sock = accept(sock, NULL, NULL);
        assert(cli_sock != INVALID_SOCKET);
        
        printf("Player %d connected\n", players[i]);
        connections[i] = connection_init(cli_sock, false);
    }

    for(int i = 0; i < 2; i++)
    {
        packet_t p = {0};
        p.kind = PACKET_KIND_RESPONSE_CONNECT;
        p.response_connect.assigned_player = players[i];
        packet_send(&connections[i], p);
    }

    game_t game = game_make(3, players[0]);

    fd_set fds;

    bool running = true;

    while(running)
    {
        FD_ZERO(&fds);

        for(size_t i = 0; i < 2; i++)
            FD_SET(connections[i].sock, &fds);
        
        if(select(0, &fds, NULL, NULL, NULL) == -1)
            break;
        
        for(size_t i = 0; i < 2; i++)
        {
            if(FD_ISSET(connections[i].sock, &fds))
            {
                packet_t p;
                packet_status_t status;
                
                while((status = packet_poll(&connections[i], &p)) == PACKET_STATUS_WAITING);

                if(status == PACKET_STATUS_NO_MORE)
                    running = false;
                else if(p.kind == PACKET_KIND_REQUEST_MOVE)
                {
                    int x = p.resquest_move.x;
                    int y = p.resquest_move.y;

                    if(players[i] == game.curr_player && game_do_move(&game, x, y))
                    {
                        printf("Player %d made move (%d, %d)\n", players[i], x, y);
                        
                        packet_t p = {0};
                        p.kind = PACKET_KIND_RESPONSE_MOVE;
                        p.response_move.player = players[i];
                        p.response_move.x = x;
                        p.response_move.y = y; 
                        
                        for(size_t j = 0; j < 2; j++)
                            packet_send(&connections[j], p);
                                
                        if(!game.running)
                        {
                            printf("Game finished\n");
                            running = false;
                        }
                    }
                }
            }
        }
    }

    game_free(&game);
    
    for(size_t i = 0; i < 2; i++)
        connection_close(&connections[i]);

    closesocket(sock);
    WSACleanup();
    return 0;
}