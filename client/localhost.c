#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ai.h"
#include "ui.h"
#include "scene.h"
#include "config.h"

#include <quartz/quartz.h>

extern const scene_t menu_scene;

static void* scene_make(scene_persistent_data_t pdata);
static void scene_update(scene_selector_t* selector, void* ctx_);
static void scene_free(void* ctx_);

const scene_t localhost_scene = {
    scene_make,
    scene_update,
    scene_free,
};

typedef struct
{
    quartz_viewport viewport;
    quartz_font font;
    const quartz_camera2D* camera;

    ui_button_t back_btn;
    ui_button_t reset_btn;

    bool successful_load;

    game_t game;
    bool is_ai [3];
} context_t;

static context_t ctx = {0};

static void* scene_make(scene_persistent_data_t pdata)
{
    ctx.viewport = pdata.viewport;
    ctx.font = pdata.font;
    ctx.camera = pdata.camera;
    
    ui_button_t back_btn = {0};
    back_btn.position = (quartz_vec2){ -400 + 60, 300 - 60 };
    back_btn.scale = (quartz_vec2){ 60, 60 };

    ui_button_t reset_btn = {0};
    reset_btn.position = (quartz_vec2){ 0, -240 };
    reset_btn.scale = (quartz_vec2){ 100, 50 };

    ctx.back_btn = back_btn;
    ctx.reset_btn = reset_btn;
    ctx.successful_load = true;
    ctx.game = game_make(3, PLAYER_1);

    if(!localhost_config_load_from_file("config/localhost.txt", &ctx.is_ai[PLAYER_1], &ctx.is_ai[PLAYER_2]))
        ctx.successful_load = false;
    
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

    ctx->reset_btn.disabled = ctx->game.running;
    ui_check_button_hover(&ctx->reset_btn, mouse_pos);

    if(ctx->successful_load)
    {
        if(!ctx->game.running)
        {
            if(ctx->reset_btn.hovered && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
                game_restart(&ctx->game);
        }
        else
        {
            int x, y;

            if(ctx->is_ai[ctx->game.curr_player])
            {
                ai_next_move(&ctx->game.board, &x, &y, ctx->game.curr_player);
                game_do_move(&ctx->game, x, y);
            }
            else if(quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
            {
                if(ui_match_point_to_board_cell(ui_info, &ctx->game.board, mouse_pos, &x, &y))
                    game_do_move(&ctx->game, x, y);
            }
        }
    }

    quartz_clear(ui_info.background_color);

    if(ctx->successful_load)
    {
        if(!ctx->game.running)
        {
            player_t winner;
            board_is_final(&ctx->game.board, &winner);

            const char* winner_text = ui_get_winner_text(winner);
            quartz_vec2 winner_text_pos = { 0, 240 };
            ui_draw_text_centered(ctx->font, 50, winner_text, winner_text_pos, UI_WHITE_COLOR);
        }

        ui_draw_board(ui_info, &ctx->game.board);

        // Draw cell highlight
        int x, y;

        if(ctx->game.running && ui_match_point_to_board_cell(ui_info, &ctx->game.board, mouse_pos, &x, &y)
        && board_get_cell(&ctx->game.board, x, y) == NO_PLAYER)
        {
            quartz_vec2 point;
            ui_match_board_cell_to_point(ui_info, &ctx->game.board, x, y, &point);
            
            quartz_color highlight_color = UI_WHITE_COLOR;
            highlight_color.a = 0.20;
            quartz_render2D_quad(highlight_color, point, (quartz_vec2){ui_info.cell_diplay_size, ui_info.cell_diplay_size}, 0.0f);
        }

        const char* player1_s = "Player 1";
        const char* player2_s = "Player 2";
    
        if(ctx->is_ai[PLAYER_1] && ctx->is_ai[PLAYER_2])
        {
            player1_s = "AI";
            player2_s = "AI";
        }
        else if(ctx->is_ai[PLAYER_1] || ctx->is_ai[PLAYER_2])
        {
            player1_s = ctx->is_ai[PLAYER_1] ? "AI" : "Player";
            player2_s = ctx->is_ai[PLAYER_2] ? "AI" : "Player";
        }
    
        ui_draw_X(UI_RED_COLOR, (quartz_vec2){-300, 0}, 100);
        ui_draw_text_centered(ctx->font, 25, player1_s, (quartz_vec2){-300, -70}, UI_WHITE_COLOR);
    
        ui_draw_O(UI_BLUE_COLOR, UI_BLACK_COLOR, (quartz_vec2){300, 0},100);
        ui_draw_text_centered(ctx->font, 25, player2_s, (quartz_vec2){300, -70}, UI_WHITE_COLOR);
    }
    else
    {
        ui_draw_text_centered(ctx->font, 30, "Config file does not exist or could not be loaded", (quartz_vec2){0}, UI_RED_COLOR);
    }
    
    ui_draw_button(&ctx->back_btn, ctx->font, 1, "", UI_BLACK_COLOR, UI_GREEN_COLOR, ui_ligthen_color(UI_GREEN_COLOR, 0.30));

    quartz_vec2 back_text_scale = { 
        -1 * 35.0f/quartz_texture_get_info(selector->pdata.arrow_texture).width,
        35.0f/quartz_texture_get_info(selector->pdata.arrow_texture).height
    };
    quartz_render2D_texture(selector->pdata.arrow_texture, ctx->back_btn.position, back_text_scale, UI_DEG2RAD * -90, QUARTZ_WHITE);
    
    ui_draw_button(&ctx->reset_btn, ctx->font, 25, "Reset", UI_BLACK_COLOR, UI_GREEN_COLOR, ui_ligthen_color(UI_GREEN_COLOR, 0.30));
    
    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;
    game_free(&ctx->game);
    memset(ctx, 0, sizeof(*ctx));
}
