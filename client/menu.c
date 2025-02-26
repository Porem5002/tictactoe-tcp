#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "scene.h"
#include "ui.h"

extern const scene_t localhost_scene;
extern const scene_t online_scene;

static void* scene_make(scene_persistent_data_t pdata);
static void scene_update(scene_selector_t* selector, void* ctx_);
static void scene_free(void* ctx_);

const scene_t menu_scene = {
    .make = scene_make,
    .update = scene_update,
    .free = scene_free
};

typedef struct
{
    quartz_viewport viewport;
    quartz_font font;
    const quartz_camera2D* camera;

    ui_text_t title_text;

    ui_button_t localhost_btn;
    ui_text_t localhost_text;

    ui_button_t online_btn;
    ui_text_t online_text;

    ui_button_t quit_btn;
    ui_text_t quit_text;

    anim_property_t anims [30];
} context_t;

static context_t ctx = {0};

static void* scene_make(scene_persistent_data_t pdata)
{
    quartz_vec2 btn_scale = { 200, 80 };
    float font_size = 24;

    ctx.title_text = (ui_text_t){
        .font = pdata.font,
        .font_size = 50,
        .text = "TicTacToe",
        .position = (quartz_vec2){0, 240},
        .color = UI_GREEN_COLOR,
    };

    ctx.localhost_btn = (ui_button_t){
        .color = UI_GREEN_COLOR,
        .position = (quartz_vec2){0, 100},
        .scale = btn_scale,
    };

    ctx.localhost_text = (ui_text_t){
        .font = pdata.font,
        .font_size = font_size,
        .text = "Localhost",
        .position = ctx.localhost_btn.position, 
        .color = UI_BLACK_COLOR,
    };

    ctx.online_btn = (ui_button_t){
        .color = UI_GREEN_COLOR,
        .position = (quartz_vec2){0, -10},
        .scale = btn_scale,
    };
    
    ctx.online_text = (ui_text_t){
        .font = pdata.font,
        .font_size = font_size,
        .text = "Online",
        .position = ctx.online_btn.position, 
        .color = UI_BLACK_COLOR,
    };

    ctx.quit_btn = (ui_button_t){
        .color = UI_GREEN_COLOR,
        .position = (quartz_vec2){0, -120},
        .scale = btn_scale,
    };

    ctx.quit_text = (ui_text_t){
        .font = pdata.font,
        .font_size = font_size,
        .text = "Quit",
        .position = ctx.quit_btn.position, 
        .color = UI_BLACK_COLOR,
    };

    anim_writer_t wr = {
        .cap = sizeof(ctx.anims) / sizeof(anim_property_t),
        .buffer = ctx.anims,
    };

    ui_fill_text_button_anims(&wr, &ctx.localhost_btn, &ctx.localhost_text);
    ui_fill_text_button_anims(&wr, &ctx.online_btn, &ctx.online_text);
    ui_fill_text_button_anims(&wr, &ctx.quit_btn, &ctx.quit_text);

    ctx.viewport = pdata.viewport;
    ctx.font = pdata.font;
    ctx.camera = pdata.camera;
    return &ctx;
}

static void scene_update(scene_selector_t* selector, void* ctx_)
{
    context_t* ctx = ctx_;

    quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
    quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(ctx->camera, mouse_screen_pos, ctx->viewport);

    if(ui_button_update(&ctx->localhost_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
        scene_selector_change(selector, localhost_scene);

    if(ui_button_update(&ctx->online_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
        scene_selector_change(selector, online_scene);

    if(ui_button_update(&ctx->quit_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
        quartz_quit();

    quartz_clear(UI_BLACK_COLOR);

    ui_text_draw(&ctx->title_text);

    ui_button_draw(&ctx->localhost_btn);
    ui_text_draw(&ctx->localhost_text);

    ui_button_draw(&ctx->online_btn);
    ui_text_draw(&ctx->online_text);

    ui_button_draw(&ctx->quit_btn);
    ui_text_draw(&ctx->quit_text);

    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;
    memset(ctx, 0, sizeof(*ctx));
}
