#include <stdlib.h>

#include "base.h"
#include "ai.h"
#include "ui.h"
#include "scene.h"

#include "../external/quartz.h"

static void* scene_make(const quartz_font* font, const quartz_camera2D* camera);
static void scene_update(scene_selector_t* selector, void* ctx_);
static void scene_free(void* ctx_);

const scene_t localhost_scene = {
    scene_make,
    scene_update,
    scene_free,
};

typedef struct
{
    const quartz_camera2D* camera;

    game_t game;
    bool is_ai [3];
} context_t;

static context_t ctx = {0};

static void* scene_make(const quartz_font* font, const quartz_camera2D* camera)
{
    ctx.camera = camera;
    ctx.game = game_make(3, PLAYER_1);
    ctx.is_ai[PLAYER_1] = true;
    ctx.is_ai[PLAYER_2] = false;
    return &ctx;
}

static void scene_update(scene_selector_t* selector, void* ctx_)
{
    context_t* ctx = ctx_;

    ui_info_t ui_info = {
        .background_color = QUARTZ_BLACK,
        .cell_color = { 0.95, 0.95, 0.95, 1.0 },
        .p1_color = QUARTZ_RED,
        .p2_color = QUARTZ_BLUE,
        .cell_diplay_size = 100,
        .symbol_display_size = 50
    };

    quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
    quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(ctx->camera, mouse_screen_pos, quartz_get_screen_viewport());

    if(!ctx->game.running && quartz_is_key_down(QUARTZ_KEY_SPACE))
        game_restart(&ctx->game);
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
    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;
    game_free(&ctx->game);
    memset(ctx, 0, sizeof(*ctx));
}
