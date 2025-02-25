#ifndef UI_H
#define UI_H

#include <stdbool.h>

#include <quartz/quartz.h>

#include "anim.h"

#include "../shared/game.h"

#define UI_DEG2RAD (3.1415f/180)
#define UI_RAD2DEG (180/3.1415f)

#define ui_rgb(R, G, B) ((quartz_color){ (R)/255.0f, (G)/255.0f, (B)/255.0f, 1.0f })
#define ui_norm_rgb(R, G, B) ((quartz_color){ (R), (G), (B), 1.0f })

#define UI_WHITE_COLOR ui_rgb(241, 247, 230)
#define UI_BLACK_COLOR ui_rgb(19, 19, 18)

#define UI_RED_COLOR ui_rgb(249, 68, 68)
#define UI_BLUE_COLOR ui_rgb(49, 151, 254)
#define UI_GREEN_COLOR ui_rgb(156, 254, 103)

typedef struct
{
    size_t anims_size;
    anim_property_t* anims;

    quartz_color color;
    quartz_vec2 position;
    quartz_vec2 scale;

    bool selected;
    bool disabled;
} ui_button_t;

typedef struct
{
    quartz_font font;
    float font_size;
    const char* text;
    quartz_vec2 position;
    quartz_color color;
} ui_text_t;

typedef struct
{
    quartz_texture texture;
    quartz_vec2 position;
    quartz_vec2 scale;
    quartz_color tint;
    float rotation;
} ui_texture_t;

typedef struct
{
    quartz_color background_color;
    quartz_color cell_color;
    quartz_color wall_color;

    quartz_color p1_color;
    quartz_color p2_color;

    float cell_diplay_size;
    float wall_diplay_size;
    float symbol_display_size;
} ui_info_t;

quartz_color ui_ligthen_color(quartz_color color, float factor);
quartz_color ui_darken_color(quartz_color color, float factor);

float ui_calc_board_side_size(ui_info_t info, const board_t* b);
quartz_vec2 ui_calc_board_origin(quartz_vec2 pos, ui_info_t info, const board_t* b);
bool ui_match_point_to_board_cell(ui_info_t info, const board_t* b, quartz_vec2 point, int* x, int* y);
bool ui_match_board_cell_to_point(ui_info_t info, const board_t* b, int x, int y, quartz_vec2* point);

const char* ui_get_winner_text(player_t winner);
void ui_draw_board(ui_info_t info, const board_t* b);
void ui_draw_X(quartz_color color, quartz_vec2 pos, float size);
void ui_draw_O(quartz_color outer_color, quartz_color inner_color, quartz_vec2 pos, float size);
void ui_text_draw_inline(quartz_font font, float font_size, const char* text, quartz_vec2 pos, quartz_color color);

quartz_aabb2 ui_button_get_aabb(const ui_button_t* btn);
bool ui_button_update(ui_button_t* btn, quartz_vec2 point);
void ui_button_draw(const ui_button_t* btn);

void ui_text_draw(const ui_text_t* text);
void ui_texture_draw(const ui_texture_t* texture);

#endif