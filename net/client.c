#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <winsock2.h>

#include "..\src\base.h"
#include "..\src\ai.h"
#include "..\src\ui.h"

#include "..\external/quartz.h"

#include "net.h"

typedef enum
{
    MODE_CONNECTING,
    MODE_ASSIGNING,
    MODE_PLAYING,

    MODE_FAILED_CONNECT,
    MODE_LOST_CONTACT,
} mode_t;

int main(void)
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    quartz_start(800, 600, "TicTacToe");

    ui_info_t ui_info = {
        .background_color = QUARTZ_BLACK,
        .cell_color = { 0.95, 0.95, 0.95, 1.0 },
        .p1_color = QUARTZ_RED,
        .p2_color = QUARTZ_BLUE,
        .cell_diplay_size = 100,
        .symbol_display_size = 50
    };
    
    mode_t mode = MODE_CONNECTING;

    quartz_font font = quartz_load_font("assets/fonts/Lato-Black.ttf");

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != INVALID_SOCKET);

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(6000);

    connection_t connection = connection_init(sock, true);
    connection_start_connect(&connection, serv_addr);
    
    player_t player = NO_PLAYER;
    board_t board = board_make(3);
    quartz_camera2D camera = quartz_init_camera2D(800, 600);

    quartz_render2D_init();
    quartz_render2D_set_camera(&camera);

    float timeout = 2;

    while(quartz_update())
    {
        quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
        quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(&camera, mouse_screen_pos, quartz_get_screen_viewport());

        switch(mode)
        {
            case MODE_CONNECTING:
            {
                connect_status_t status = connection_continue_connect(&connection);

                if(status == CONNECT_STATUS_SUCCESS)
                    mode++;
                else if(status == CONNECT_STATUS_FAILED || timeout <= 0)
                {
                    mode = MODE_FAILED_CONNECT;
                    break;
                }

                timeout -= quartz_get_delta_time();
            }
            break;

            case MODE_ASSIGNING:
            {
                packet_t p;
                packet_status_t status = packet_poll(&connection, &p);

                if(status == PACKET_STATUS_RECEIVED)
                {
                    assert(p.kind == PACKET_KIND_RESPONSE_CONNECT);
                    player = p.response_connect.assigned_player;
                    printf("Connected as player %d\n", player);
                    mode++;
                }
                else if(status == PACKET_STATUS_NO_MORE)
                {
                    mode = MODE_LOST_CONTACT;
                    break;
                }
            }
            break;

            case MODE_PLAYING:
            {
                packet_t p;
                packet_status_t status;

                while((status = packet_poll(&connection, &p)) == PACKET_STATUS_RECEIVED)
                {   
                    if(p.kind == PACKET_KIND_RESPONSE_MOVE)
                        board_set_cell(&board, p.response_move.x, p.response_move.y, p.response_move.player);
                }

                if(status == PACKET_STATUS_NO_MORE)
                {
                    mode = MODE_LOST_CONTACT;
                    break;
                }

                int x, y;
                
                if(quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN) && ui_match_point_to_board_cell(ui_info, &board, mouse_pos, &x, &y))
                {
                    packet_t p = {0};
                    p.kind = PACKET_KIND_REQUEST_MOVE;
                    p.resquest_move.x = x;
                    p.resquest_move.y = y;
                    
                    if(!packet_send(&connection, p))
                    {
                        mode = MODE_LOST_CONTACT;
                        break;
                    }
                }
            }
            break;
            default:
                break;
        }

        quartz_clear(ui_info.background_color);
        
        float font_size = 30;
        quartz_vec2 text_size;
        const char* text; 

        switch(mode)
        {
            case MODE_CONNECTING:
            {
                text = "Connecting to the server...";
                text_size = quartz_font_get_text_size(font, font_size, text);
                quartz_vec2 pos = { -text_size.x/2, text_size.y/2 };
                quartz_render2D_text(font, font_size, text, pos, QUARTZ_WHITE);
            }
            break;
            case MODE_ASSIGNING:
            {
                text = "Waiting for another player...";
                text_size = quartz_font_get_text_size(font, font_size, text);
                quartz_vec2 pos = { -text_size.x/2, text_size.y/2 };
                quartz_render2D_text(font, font_size, text, pos, QUARTZ_WHITE);
            }
            break;
            case MODE_PLAYING:
                ui_draw_board(ui_info, &board);
                break;
            case MODE_FAILED_CONNECT:
            {
                text = "Could not connect to the server!";
                text_size = quartz_font_get_text_size(font, font_size, text);
                quartz_vec2 pos = { -text_size.x/2, text_size.y/2 };
                quartz_render2D_text(font, font_size, text, pos, QUARTZ_RED);
            }
            break;
            case MODE_LOST_CONTACT:
            {
                text = "Lost contact with the server!";
                text_size = quartz_font_get_text_size(font, font_size, text);
                quartz_vec2 pos = { -text_size.x/2, text_size.y/2 };
                quartz_render2D_text(font, font_size, text, pos, QUARTZ_RED);
            }
            break;
        }
        
        quartz_render2D_flush();
    }

    connection_close(&connection);
    board_free(&board);
    quartz_finish();
    WSACleanup();
    return 0;
}
