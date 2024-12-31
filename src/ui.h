#ifndef UI_H
#define UI_H

#include <stdbool.h>

#include "base.h"

#include "../external/quartz.h"

#define ui_rgb(R, G, B) ((quartz_color){ (R)/255.0f, (G)/255.0f, (B)/255.0f, 1.0f })
#define ui_norm_rgb(R, G, B) ((quartz_color){ (R), (G), (B), 1.0f })

#define UI_WHITE_COLOR ui_rgb(241, 247, 230)
#define UI_BLACK_COLOR ui_rgb(19, 19, 18)

#define UI_RED_COLOR ui_rgb(249, 68, 68)
#define UI_BLUE_COLOR ui_rgb(49, 151, 254)
#define UI_GREEN_COLOR ui_rgb(156, 254, 103)

typedef struct
{
    bool disabled;
    bool hovered;
    quartz_vec2 position;
    quartz_vec2 scale;
} ui_button_t;

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
void ui_draw_board(ui_info_t info, const board_t* b);

void ui_draw_X(quartz_color color, quartz_vec2 pos, float size);
void ui_draw_O(quartz_color outer_color, quartz_color inner_color, quartz_vec2 pos, float size);

void ui_draw_text_centered(quartz_font font, float font_size, const char* text, quartz_vec2 pos, quartz_color color);

quartz_aabb2 ui_get_button_aabb(const ui_button_t* btn);
bool ui_check_button_hover(ui_button_t* btn, quartz_vec2 point);
void ui_draw_button(const ui_button_t* btn, quartz_font font, float font_size, const char* text, quartz_color text_color, quartz_color base_color, quartz_color hover_color);

#endif