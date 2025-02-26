#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "ai.h"
#include "ui.h"
#include "scene.h"
#include "config.h"
#include "anim.h"

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
    ui_texture_t back_texture;

    ui_button_t reset_btn;
    ui_text_t reset_text;

    bool successful_load;

    game_t game;
    bool is_ai [3];

    anim_property_t player1_my_turn_anim;
    anim_property_t player2_my_turn_anim;
    anim_property_t anims [13];
} context_t;

static context_t ctx = {0};

static const char* get_player_text(player_t player, bool is_player1_ai, bool is_player2_ai);
static const char* get_winner_text(player_t player, bool is_player1_ai, bool is_player2_ai);

static void* scene_make(scene_persistent_data_t pdata)
{
    ctx.back_btn = (ui_button_t){
        .color = UI_GREEN_COLOR,
        .position = (quartz_vec2){ -400 + 60, 300 - 60 },
        .scale = (quartz_vec2){ 60, 60 },
    };

    ctx.back_texture = (ui_texture_t){
        .texture = pdata.arrow_texture,
        .position = (quartz_vec2){ -400 + 60, 300 - 60 },
        .scale = (quartz_vec2){ 35, 35 },
        .tint = UI_WHITE_COLOR,
        .rotation = UI_DEG2RAD * -90,
    };

    ctx.reset_btn = (ui_button_t){
        .color = UI_GREEN_COLOR,
        .position = (quartz_vec2){ 0, -240 },
        .scale = (quartz_vec2){ 100, 50 },
    };

    ctx.reset_text = (ui_text_t){
        .font = pdata.font,
        .font_size = 24,
        .text = "Reset",
        .color = UI_BLACK_COLOR,
        .position = ctx.reset_btn.position,
    };

    anim_writer_t wr = {
        .cap = sizeof(ctx.anims) / sizeof(anim_property_t),
        .buffer = ctx.anims,
    };

    ui_fill_texture_button_anims(&wr, &ctx.back_btn, &ctx.back_texture);
    ui_fill_text_button_anims(&wr, &ctx.reset_btn, &ctx.reset_text);

    ctx.player1_my_turn_anim = ui_get_player_my_turn_anim();
    ctx.player2_my_turn_anim = ui_get_player_my_turn_anim();

    ctx.successful_load = true;
    ctx.game = game_make(3, PLAYER_1);

    if(!localhost_config_load_from_file("config/localhost.txt", &ctx.is_ai[PLAYER_1], &ctx.is_ai[PLAYER_2]))
        ctx.successful_load = false;
    
    ctx.viewport = pdata.viewport;
    ctx.font = pdata.font;
    ctx.camera = pdata.camera;
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

    ctx->reset_btn.disabled = ctx->game.running;
    ui_button_update(&ctx->reset_btn, mouse_pos);

    if(ui_button_update(&ctx->back_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
        scene_selector_change(selector, menu_scene);

    if(ctx->successful_load)
    {
        if(!ctx->game.running)
        {
            if(ctx->reset_btn.selected && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
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
        
            if(ctx->game.curr_player == PLAYER_1 &&
               anim_property_update(&ctx->player1_my_turn_anim, quartz_get_delta_time()) == ANIM_STATUS_FINISHED)
                anim_property_reset(&ctx->player1_my_turn_anim);

            if(ctx->game.curr_player == PLAYER_2 &&
               anim_property_update(&ctx->player2_my_turn_anim, quartz_get_delta_time()) == ANIM_STATUS_FINISHED)
                anim_property_reset(&ctx->player2_my_turn_anim);
        }
    }

    quartz_clear(ui_info.background_color);

    if(ctx->successful_load)
    {
        if(!ctx->game.running)
        {
            player_t winner;
            board_is_final(&ctx->game.board, &winner);

            const char* winner_text = get_winner_text(winner, ctx->is_ai[PLAYER_1], ctx->is_ai[PLAYER_2]);
            quartz_vec2 winner_text_pos = { 0, 240 };
            ui_text_draw_inline(ctx->font, 45, winner_text, winner_text_pos, UI_WHITE_COLOR);
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

        const char* player1_s = get_player_text(PLAYER_1, ctx->is_ai[PLAYER_1], ctx->is_ai[PLAYER_2]);
        const char* player2_s = get_player_text(PLAYER_2, ctx->is_ai[PLAYER_1], ctx->is_ai[PLAYER_2]);
    
        ui_draw_X(UI_RED_COLOR, (quartz_vec2){-300, 0}, anim_property_get_value(&ctx->player1_my_turn_anim));
        ui_text_draw_inline(ctx->font, 25, player1_s, (quartz_vec2){-300, -70}, UI_WHITE_COLOR);
    
        ui_draw_O(UI_BLUE_COLOR, UI_BLACK_COLOR, (quartz_vec2){300, 0}, anim_property_get_value(&ctx->player2_my_turn_anim));
        ui_text_draw_inline(ctx->font, 25, player2_s, (quartz_vec2){300, -70}, UI_WHITE_COLOR);
    }
    else
    {
        ui_text_draw_inline(ctx->font, 30, "Config file does not exist or could not be loaded", (quartz_vec2){0}, UI_RED_COLOR);
    }
    
    ui_button_draw(&ctx->back_btn);
    ui_texture_draw(&ctx->back_texture);

    ui_button_draw(&ctx->reset_btn);
    if(!ctx->reset_btn.disabled)
        ui_text_draw(&ctx->reset_text);

    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;
    game_free(&ctx->game);
    memset(ctx, 0, sizeof(*ctx));
}

#define gen_player_to_string_func(NAME, STR_SUFFIX, ALLOW_NO_PLAYER, STR_NO_PLAYER)\
const char* NAME(player_t player, bool is_player1_ai, bool is_player2_ai)\
{\
    if((ALLOW_NO_PLAYER) && player == NO_PLAYER) return STR_NO_PLAYER;\
    \
    assert(player == PLAYER_1 || player == PLAYER_2);\
    \
    if(is_player1_ai && is_player2_ai)\
    {\
        if(player == PLAYER_1) return "AI 1" STR_SUFFIX;\
        if(player == PLAYER_2) return "AI 2" STR_SUFFIX;\
        assert(false);\
    }\
    \
    if(!is_player1_ai && !is_player2_ai)\
    {\
        if(player == PLAYER_1) return "Player 1" STR_SUFFIX;\
        if(player == PLAYER_2) return "Player 2" STR_SUFFIX;\
        assert(false);\
    }\
    \
    if((is_player1_ai && player == PLAYER_1) || (is_player2_ai && player == PLAYER_2))\
        return "AI" STR_SUFFIX;\
    \
    return "Player" STR_SUFFIX;\
}

static gen_player_to_string_func(get_player_text, "", false, "")
static gen_player_to_string_func(get_winner_text, " Won", true, "Draw")