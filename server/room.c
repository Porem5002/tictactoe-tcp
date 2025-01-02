#include <assert.h>
#include <stdio.h>

#include "room.h"

room_t room_make(connection_t cli_conn)
{
    room_t room = {0};
    room.mode = ROOM_MODE_SETUP;
    room.conns[0] = cli_conn;
    return room;
}

void room_connect_second(room_t* room, connection_t cli_conn)
{
    room->mode = ROOM_MODE_PLAYING;
    room->conns[1] = cli_conn;
    room->game = game_make(3, PLAYER_1);

    packet_t p = {0};
    p.kind = PACKET_KIND_RESPONSE_CONNECT;

    for(int i = 0; i < 2; i++)
    {
        p.response_connect.assigned_player = PLAYER_1 + i;
        packet_send(&room->conns[i], p);
    }
}

void room_fd_set(room_t* room, fd_set* fds)
{
    FD_SET(room->conns[0].sock, fds);

    if(room->mode != ROOM_MODE_SETUP)
        FD_SET(room->conns[1].sock, fds);
}

bool room_update(room_t* room, fd_set* fds)
{
    switch(room->mode)
    {
        case ROOM_MODE_SETUP:
            {
                packet_t p;

                if(FD_ISSET(room->conns[0].sock, fds) && packet_poll(&room->conns[0], &p) == PACKET_STATUS_NO_MORE)
                    return false;
            }
            break;
        case ROOM_MODE_PLAYING:
            for(int i = 0; i < 2; i++)
            {
                player_t player = PLAYER_1 + i;
                if(!FD_ISSET(room->conns[i].sock, fds)) continue;

                packet_t p;
                packet_status_t status = packet_poll(&room->conns[i], &p);
            
                if(status == PACKET_STATUS_RECEIVED && p.kind == PACKET_KIND_REQUEST_MOVE)
                {
                    int x = p.resquest_move.x;
                    int y = p.resquest_move.y;

                    if(room->game.curr_player == player && game_do_move(&room->game, x, y))
                    {
                        packet_t p = {0};
                        p.kind = PACKET_KIND_RESPONSE_MOVE;
                        p.response_move.player = player;
                        p.response_move.x = x;
                        p.response_move.y = y;

                        packet_send(&room->conns[0], p);
                        packet_send(&room->conns[1], p);

                        if(!room->game.running)
                        {
                            player_t player;
                            board_is_final(&room->game.board, &player);

                            p.kind = PACKET_KIND_RESPONSE_WINNER;
                            p.response_winner.player = player;

                            packet_send(&room->conns[0], p);
                            packet_send(&room->conns[1], p);
                            
                            room->mode = ROOM_MODE_FINISHED;
                            return true;
                        }
                    }
                }
                
                if(status == PACKET_STATUS_NO_MORE) return false;
            }
            break;
        case ROOM_MODE_FINISHED:
            for(int i = 0; i < 2; i++)
            {
                if(!FD_ISSET(room->conns[i].sock, fds)) continue;

                packet_t p;
                packet_status_t status = packet_poll(&room->conns[i], &p);

                if(status == PACKET_STATUS_NO_MORE) return false;
            }
            break;
        default:
            assert(false);
    }

    return true;
}

void room_free(room_t* room)
{
    connection_close(&room->conns[0]);

    if(room->mode != ROOM_MODE_SETUP)
    {
        connection_close(&room->conns[1]);
        game_free(&room->game);
    }
}