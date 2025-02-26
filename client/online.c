#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <winsock2.h>

#include "ai.h"
#include "ui.h"
#include "scene.h"
#include "config.h"

#include "../shared/net.h"

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
    MODE_FINISHED,

    MODE_FAILED_CONNECT,
    MODE_LOST_CONTACT,
} mode_t;

typedef struct
{
    quartz_viewport viewport;
    quartz_font font;
    const quartz_camera2D* camera;

    ui_button_t back_btn;
    ui_texture_t back_texture;

    ui_button_t reset_btn;
    ui_text_t reset_text;

    char* server_name;
    unsigned short server_port;

    mode_t mode;
    float conn_timeout;

    connection_t conn;
    player_t player;
    board_t board;
    player_t winner;

    anim_property_t anims [13];
} context_t;

static context_t ctx;

static const char* get_player_text(player_t player, player_t me);
static const char* get_winner_text(player_t player, player_t me);

static void* scene_make(scene_persistent_data_t pdata)
{
    anim_writer_t wr = {
        .cap = sizeof(ctx.anims) / sizeof(anim_property_t),
        .buffer = ctx.anims,
    };

    anim_t base_anim = { .duration = 0.15 };

    ctx.back_btn = (ui_button_t){
        .color = UI_GREEN_COLOR,
        .position = (quartz_vec2){ -400 + 60, 300 - 60 },
        .scale = (quartz_vec2){ 60, 60 },
    };

    ctx.back_texture = (ui_texture_t){
        .texture = pdata.arrow_texture,
        .position = ctx.back_btn.position,
        .scale = { 35.0f, 35.0f },
        .tint = QUARTZ_WHITE,
        .rotation = UI_DEG2RAD * -90.0f,
    };

    anim_writer_write_vec2(&wr, base_anim, ctx.back_btn.scale, (quartz_vec2){ctx.back_btn.scale.x + 10,ctx.back_btn.scale.y + 10}, &ctx.back_btn.scale);
    anim_writer_write_color3(&wr, base_anim, ctx.back_btn.color, ui_ligthen_color(ctx.back_btn.color, 0.30), &ctx.back_btn.color);
    anim_writer_write_vec2(&wr, base_anim, ctx.back_texture.scale, (quartz_vec2){ctx.back_texture.scale.x + 5,ctx.back_texture.scale.y + 5}, &ctx.back_texture.scale);

    ctx.back_btn.anims_size = anim_writer_get_size(&wr);
    ctx.back_btn.anims = anim_writer_get_baseptr(&wr);

    ctx.reset_btn = (ui_button_t){
        .color = UI_GREEN_COLOR,
        .position = (quartz_vec2){ 0, -240 },
        .scale = (quartz_vec2){ 100, 50 },
    };

    ctx.reset_text = (ui_text_t){
        .font = pdata.font,
        .font_size = 25,
        .text = "Reset",
        .position = ctx.reset_btn.position,
        .color = UI_BLACK_COLOR,
    };

    anim_writer_rebase(&wr);

    anim_writer_write_vec2(&wr, base_anim, ctx.reset_btn.scale, (quartz_vec2){ctx.reset_btn.scale.x+10,ctx.reset_btn.scale.y+10}, &ctx.reset_btn.scale);
    anim_writer_write_color3(&wr, base_anim, ctx.reset_btn.color, ui_ligthen_color(ctx.reset_btn.color, 0.30), &ctx.reset_btn.color);
    anim_writer_write_float(&wr, base_anim, ctx.reset_text.font_size, ctx.reset_text.font_size + 2, &ctx.reset_text.font_size);

    ctx.reset_btn.anims_size = anim_writer_get_size(&wr);
    ctx.reset_btn.anims = anim_writer_get_baseptr(&wr);

    ctx.viewport = pdata.viewport;
    ctx.font = pdata.font;
    ctx.camera = pdata.camera;

    ctx.mode = MODE_CONNECTING;
    ctx.conn_timeout = 2;
    ctx.player = NO_PLAYER;
    ctx.board = board_make(3);
    ctx.winner = NO_PLAYER;

    if(!online_config_load_from_file("config/online.txt", &ctx.server_name, &ctx.server_port))
    {
        ctx.mode = MODE_FAILED_CONNECT;
        return &ctx;
    }

    IN_ADDR server_ip;

    if(!net_get_ip_fom_name(ctx.server_name, &server_ip))
    {
        ctx.mode = MODE_FAILED_CONNECT;
        return &ctx;
    }
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != INVALID_SOCKET);

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = server_ip;
    serv_addr.sin_port = htons(ctx.server_port);
    
    connection_t conn = connection_init(sock, true);
    connection_start_connect(&conn, serv_addr);

    ctx.conn = conn;

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

    ctx->reset_btn.disabled = ctx->mode != MODE_FINISHED;

    if(ui_button_update(&ctx->back_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
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

                if(p.kind == PACKET_KIND_RESPONSE_WINNER)
                {
                    ctx->mode = MODE_FINISHED;
                    ctx->winner = p.response_winner.player;
                }
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

            // TODO: Add player symbol animations according to current player
        }
        break;
        case MODE_FINISHED:
        {
            packet_t p;
            packet_status_t status;

            while((status = packet_poll(&ctx->conn, &p)) == PACKET_STATUS_RECEIVED)
            {
                if(p.kind == PACKET_KIND_RESPONSE_RESET)
                {
                    ctx->mode = MODE_PLAYING;
                    board_clear(&ctx->board);
                }
            }

            if(ui_button_update(&ctx->reset_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
            {
                packet_t p = {0};
                p.kind = PACKET_KIND_REQUEST_RESET;
                packet_send(&ctx->conn, p);
            }

            if(status == PACKET_STATUS_NO_MORE)
                ctx->mode = MODE_LOST_CONTACT;
        }
        break;
        default: break;
    }

    quartz_clear(ui_info.background_color);
    
    if(ctx->mode == MODE_PLAYING)
    {
        ui_draw_board(ui_info, &ctx->board);

        // Draw cell highlight
        int x, y;

        if(ui_match_point_to_board_cell(ui_info, &ctx->board, mouse_pos, &x, &y)
           && board_get_cell(&ctx->board, x, y) == NO_PLAYER)
        {
            quartz_vec2 point;
            ui_match_board_cell_to_point(ui_info, &ctx->board, x, y, &point);
            
            quartz_color highlight_color = UI_WHITE_COLOR;
            highlight_color.a = 0.20;
            quartz_render2D_quad(highlight_color, point, (quartz_vec2){ui_info.cell_diplay_size, ui_info.cell_diplay_size}, 0.0f);
        }
    }
    else if(ctx->mode == MODE_FINISHED)
    {
        const char* winner_text = get_winner_text(ctx->winner, ctx->player);
        quartz_vec2 winner_text_pos = {0, 240};
        ui_text_draw_inline(ctx->font, 45, winner_text, winner_text_pos, UI_WHITE_COLOR);
        ui_draw_board(ui_info, &ctx->board);
    }
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
        ui_text_draw_inline(ctx->font, 30, text, pos, color);
    }

    if(ctx->mode == MODE_PLAYING || ctx->mode == MODE_FINISHED)
    {
        const char* player1_s = get_player_text(PLAYER_1, ctx->player);
        const char* player2_s = get_player_text(PLAYER_2, ctx->player);

        ui_draw_X(UI_RED_COLOR, (quartz_vec2){-300, 0}, 100);
        ui_text_draw_inline(ctx->font, 25, player1_s, (quartz_vec2){-300, -70}, UI_WHITE_COLOR);

        ui_draw_O(UI_BLUE_COLOR, UI_BLACK_COLOR, (quartz_vec2){300, 0}, 100);
        ui_text_draw_inline(ctx->font, 25, player2_s, (quartz_vec2){300, -70}, UI_WHITE_COLOR);
    }

    ui_button_draw(&ctx->back_btn);
    ui_texture_draw(&ctx->back_texture);

    ui_button_draw(&ctx->reset_btn);
    ui_text_draw(&ctx->reset_text);

    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;

    connection_close(&ctx->conn);
    board_free(&ctx->board);
    free(ctx->server_name);
    memset(ctx, 0, sizeof(*ctx));
}

#define gen_player_to_string_func(NAME, STR_SUFFIX, ALLOW_NO_PLAYER, STR_NO_PLAYER)\
const char* NAME(player_t player, player_t me)\
{\
    if((ALLOW_NO_PLAYER) && player == NO_PLAYER) return STR_NO_PLAYER;\
    \
    assert(player == PLAYER_1 || player == PLAYER_2);\
    \
    return player == me ? "You" STR_SUFFIX : "Opponent" STR_SUFFIX;\
}

static gen_player_to_string_func(get_player_text, "", false, "")
static gen_player_to_string_func(get_winner_text, " Won", true, "Draw")