#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <winsock2.h>

#include "base.h"
#include "ai.h"
#include "ui.h"
#include "scene.h"

#include "..\external/quartz.h"

#include "..\net\net.h"

extern const scene_t menu_scene;

static void* scene_make(scene_persistent_data_t pdata);
static void scene_update(scene_selector_t* selector, void* ctx_);
static void scene_free(void* ctx_); 

const scene_t online_scene = {
    scene_make,
    scene_update,
    scene_free,
};

typedef enum
{
    MODE_CONNECTING,
    MODE_ASSIGNING,
    MODE_PLAYING,

    MODE_FAILED_CONNECT,
    MODE_LOST_CONTACT,
} mode_t;

typedef struct
{
    quartz_viewport viewport;
    quartz_font font;
    const quartz_camera2D* camera;

    ui_button_t back_btn;

    mode_t mode;
    float conn_timeout;
    connection_t conn;
    player_t player;
    board_t board;
} context_t;

static context_t ctx;

static void* scene_make(scene_persistent_data_t pdata)
{
    ui_button_t back_btn = {0};
    back_btn.position = (quartz_vec2){ -400 + 60, 300 - 60 };
    back_btn.scale = (quartz_vec2){ 60, 60 };

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != INVALID_SOCKET);

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(7777);
    
    connection_t conn = connection_init(sock, true);
    connection_start_connect(&conn, serv_addr);

    ctx.viewport = pdata.viewport;
    ctx.font = pdata.font;
    ctx.camera = pdata.camera;

    ctx.back_btn = back_btn;

    ctx.mode = MODE_CONNECTING;
    ctx.conn_timeout = 2;
    ctx.conn = conn;
    ctx.player = NO_PLAYER;
    ctx.board = board_make(3);

    return &ctx;
}

static void scene_update(scene_selector_t* selector, void* ctx_)
{
    context_t* ctx = ctx_;

    ui_info_t ui_info = {
        .background_color = UI_BLACK_COLOR,
        .wall_color = UI_WHITE_COLOR,
        .cell_color = UI_BLACK_COLOR,
        .p1_color = UI_RED_COLOR,
        .p2_color = UI_BLUE_COLOR,
        .cell_diplay_size = 100,
        .wall_diplay_size = 10,
        .symbol_display_size = 50
    };

    quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
    quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(ctx->camera, mouse_screen_pos, ctx->viewport);

    if(ui_check_button_hover(&ctx->back_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
        scene_selector_change(selector, menu_scene);

    switch(ctx->mode)
    {
        case MODE_CONNECTING:
        {
            connect_status_t status = connection_continue_connect(&ctx->conn);

            if(status == CONNECT_STATUS_SUCCESS)
                ctx->mode = MODE_ASSIGNING;
            else if(status == CONNECT_STATUS_FAILED || ctx->conn_timeout <= 0)
            {
                ctx->mode = MODE_FAILED_CONNECT;
                break;
            }

            ctx->conn_timeout -= quartz_get_delta_time();
        }
        break;

        case MODE_ASSIGNING:
        {
            packet_t p;
            packet_status_t status = packet_poll(&ctx->conn, &p);

            if(status == PACKET_STATUS_RECEIVED)
            {
                assert(p.kind == PACKET_KIND_RESPONSE_CONNECT);
                ctx->player = p.response_connect.assigned_player;
                ctx->mode = MODE_PLAYING;
            }
            else if(status == PACKET_STATUS_NO_MORE)
            {
                ctx->mode = MODE_LOST_CONTACT;
                break;
            }
        }
        break;

        case MODE_PLAYING:
        {
            packet_t p;
            packet_status_t status;

            while((status = packet_poll(&ctx->conn, &p)) == PACKET_STATUS_RECEIVED)
            {   
                if(p.kind == PACKET_KIND_RESPONSE_MOVE)
                    board_set_cell(&ctx->board, p.response_move.x, p.response_move.y, p.response_move.player);
            }

            if(status == PACKET_STATUS_NO_MORE)
            {
                ctx->mode = MODE_LOST_CONTACT;
                break;
            }

            int x, y;
            
            if(quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN) && ui_match_point_to_board_cell(ui_info, &ctx->board, mouse_pos, &x, &y))
            {
                packet_t p = {0};
                p.kind = PACKET_KIND_REQUEST_MOVE;
                p.resquest_move.x = x;
                p.resquest_move.y = y;
                
                if(!packet_send(&ctx->conn, p))
                {
                    ctx->mode = MODE_LOST_CONTACT;
                    break;
                }
            }
        }
        break;
        default: break;
    }

    quartz_clear(ui_info.background_color);
    
    if(ctx->mode == MODE_PLAYING)
        ui_draw_board(ui_info, &ctx->board);
    else
    {
        const char* text;
        quartz_color color;
    
        if(ctx->mode == MODE_CONNECTING)
        {
            text = "Connecting to the server...";
            color = UI_WHITE_COLOR;
        }
        else if(ctx->mode == MODE_ASSIGNING)
        {
            text = "Waiting for another player...";
            color = UI_WHITE_COLOR;
        }
        else if(ctx->mode == MODE_FAILED_CONNECT)
        {
            text = "Could not connect to the server!";
            color = UI_RED_COLOR;
        }
        else if(ctx->mode == MODE_LOST_CONTACT)
        {
            text = "Lost contact with the server!";
            color = UI_RED_COLOR;
        }

        quartz_vec2 pos = {0};
        ui_draw_text_centered(ctx->font, 30, text, pos, color);
    }

    ui_draw_button(&ctx->back_btn, ctx->font, 35, "<", UI_BLACK_COLOR, UI_GREEN_COLOR, ui_ligthen_color(UI_GREEN_COLOR, 0.30));
    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;

    connection_close(&ctx->conn);
    board_free(&ctx->board);
    memset(ctx, 0, sizeof(*ctx));
}
