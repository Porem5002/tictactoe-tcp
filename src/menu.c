#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "scene.h"
#include "../external/quartz.h"

extern const scene_t localhost_scene;

static void* scene_make(const quartz_font* font, const quartz_camera2D* camera);
static void scene_update(scene_selector_t* selector, void* ctx_);
static void scene_free(void* ctx_);

const scene_t menu_scene = {
    .make = scene_make,
    .update = scene_update,
    .free = scene_free
};

typedef struct
{
    bool hovered;
    quartz_vec2 position;
    quartz_vec2 scale;
} button_t;

typedef struct
{
    const quartz_font* font;
    const quartz_camera2D* camera;

    button_t localhost_btn;
    button_t online_btn;
} context_t;

static context_t ctx = {0};

static quartz_aabb2 get_button_aabb(const button_t* btn);
static void draw_button(const button_t* btn, quartz_font font, float font_size, const char* text, quartz_color base_color, quartz_color hover_color);

static void* scene_make(const quartz_font* font, const quartz_camera2D* camera)
{
    quartz_vec2 btn_scale = { 300, 80 };

    button_t localhost_btn = {0};
    localhost_btn.position = (quartz_vec2){0, 60};
    localhost_btn.scale = btn_scale;

    button_t online_btn = {0};
    online_btn.position = (quartz_vec2){0, -60};
    online_btn.scale = btn_scale;

    ctx.font = font;
    ctx.camera = camera;
    ctx.localhost_btn = localhost_btn;
    ctx.online_btn = online_btn;
    return &ctx;
}

static void scene_update(scene_selector_t* selector, void* ctx_)
{
    context_t* ctx = ctx_;
    float font_size = 35;

    quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
    quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(ctx->camera, mouse_screen_pos, quartz_get_screen_viewport());

    ctx->localhost_btn.hovered = quartz_aabb2_touches_point(mouse_pos, get_button_aabb(&ctx->localhost_btn));
    ctx->online_btn.hovered = quartz_aabb2_touches_point(mouse_pos, get_button_aabb(&ctx->online_btn));

    if(quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN) && ctx->localhost_btn.hovered)
        scene_selector_change(selector, localhost_scene);

    if(quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN) && ctx->online_btn.hovered)
        printf("I want to play online\n");

    quartz_clear(QUARTZ_BLACK);

    draw_button(&ctx->localhost_btn, *ctx->font, font_size, "Localhost", QUARTZ_WHITE, QUARTZ_GREEN);
    draw_button(&ctx->online_btn, *ctx->font, font_size, "Online", QUARTZ_WHITE, QUARTZ_GREEN);

    quartz_render2D_flush();
}

static void scene_free(void* ctx_)
{
    context_t* ctx = ctx_;
    memset(ctx, 0, sizeof(*ctx));
}

static quartz_aabb2 get_button_aabb(const button_t* btn)
{
    quartz_aabb2 box = { .x = btn->position.x, .y = btn->position.y, .hwidth = btn->scale.x/2, .hheight = btn->scale.y/2 };
    return box;
}

static void draw_button(const button_t* btn, quartz_font font, float font_size, const char* text, quartz_color base_color, quartz_color hover_color)
{
    quartz_render2D_quad(!btn->hovered ? base_color : hover_color, btn->position, btn->scale, 0.0f);
    quartz_vec2 text_size = quartz_font_get_text_size(font, font_size, text);
    
    quartz_vec2 text_pos = btn->position;
    text_pos.x -= text_size.x/2;
    text_pos.y += text_size.y/2;
    quartz_render2D_text(font, font_size, text, text_pos, QUARTZ_BLACK);
}