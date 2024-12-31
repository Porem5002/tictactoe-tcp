#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "scene.h"
#include "../external/quartz.h"
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

    ui_button_t localhost_btn;
    ui_button_t online_btn;
} context_t;

static context_t ctx = {0};

static void* scene_make(scene_persistent_data_t pdata)
{
    quartz_vec2 btn_scale = { 300, 80 };

    ui_button_t localhost_btn = {0};
    localhost_btn.position = (quartz_vec2){0, 60};
    localhost_btn.scale = btn_scale;

    ui_button_t online_btn = {0};
    online_btn.position = (quartz_vec2){0, -60};
    online_btn.scale = btn_scale;

    ctx.viewport = pdata.viewport;
    ctx.font = pdata.font;
    ctx.camera = pdata.camera;
    ctx.localhost_btn = localhost_btn;
    ctx.online_btn = online_btn;
    return &ctx;
}

static void scene_update(scene_selector_t* selector, void* ctx_)
{
    context_t* ctx = ctx_;
    float font_size = 35;

    quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
    quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(ctx->camera, mouse_screen_pos, ctx->viewport);

    if(ui_check_button_hover(&ctx->localhost_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
        scene_selector_change(selector, localhost_scene);

    if(ui_check_button_hover(&ctx->online_btn, mouse_pos) && quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
        scene_selector_change(selector, online_scene);

    quartz_clear(UI_BLACK_COLOR);

    ui_draw_button(&ctx->localhost_btn, ctx->font, font_size, ">  Localhost  <", UI_BLACK_COLOR, UI_GREEN_COLOR, ui_ligthen_color(UI_GREEN_COLOR, 0.30));
    ui_draw_button(&ctx->online_btn, ctx->font, font_size, ">  Online  <", UI_BLACK_COLOR, UI_GREEN_COLOR, ui_ligthen_color(UI_GREEN_COLOR, 0.30));

    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;
    memset(ctx, 0, sizeof(*ctx));
}
