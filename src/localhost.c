#include <stdlib.h>

#include "base.h"
#include "ai.h"
#include "ui.h"
#include "scene.h"

#include "../external/quartz.h"

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
    reset_btn.position = (quartz_vec2){ 0, -200 };
    reset_btn.scale = (quartz_vec2){ 200, 60 };

    ctx.back_btn = back_btn;
    ctx.reset_btn = reset_btn;
    ctx.game = game_make(3, PLAYER_1);
    ctx.is_ai[PLAYER_1] = true;
    ctx.is_ai[PLAYER_2] = false;
    return &ctx;
}

static void scene_update(scene_selector_t* selector, void* ctx_)
{
    context_t* ctx = ctx_;
    float font_size = 35;

    ui_info_t ui_info = {
        .background_color = QUARTZ_BLACK,
        .cell_color = { 0.95, 0.95, 0.95, 1.0 },
        .p1_color = QUARTZ_RED,
        .p2_color = QUARTZ_BLUE,
        .cell_diplay_size = 100,
        .symbol_display_size = 50
    };

    quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
    quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(ctx->camera, mouse_screen_pos, ctx->viewport);

    if(ui_check_button_hover(&ctx->back_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
        scene_selector_change(selector, menu_scene);

    ctx->reset_btn.disabled = ctx->game.running;
    ui_check_button_hover(&ctx->reset_btn, mouse_pos);

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

    quartz_clear(ui_info.background_color);

    ui_draw_board(ui_info, &ctx->game.board);
    ui_draw_button(&ctx->back_btn, ctx->font, font_size, "<", QUARTZ_WHITE, QUARTZ_GREEN, (quartz_color){0.5, 0.5, 0.5, 1.0});
    ui_draw_button(&ctx->reset_btn, ctx->font, font_size, "Reset", QUARTZ_WHITE, QUARTZ_GREEN, (quartz_color){0.5, 0.5, 0.5, 1.0});
    
    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;
    game_free(&ctx->game);
    memset(ctx, 0, sizeof(*ctx));
}
