#include <assert.h>
#include <math.h>

#include "ui.h"

quartz_color ui_ligthen_color(quartz_color color, float factor)
{
    return ui_norm_rgb(color.r + factor * (1.0f - color.r), color.g + factor * (1.0f - color.g), color.b + factor * (1.0f - color.b));
}

quartz_color ui_darken_color(quartz_color color, float factor)
{
    return ui_norm_rgb(color.r - (color.r * factor), color.g - (color.g * factor), color.b - (color.b * factor));
}

float ui_calc_board_side_size(ui_info_t info, const board_t* b)
{
    return info.cell_diplay_size * b->size + info.wall_diplay_size * (b->size - 1);
}

quartz_vec2 ui_calc_board_origin(quartz_vec2 pos, ui_info_t info, const board_t* b)
{
    float side_size = ui_calc_board_side_size(info, b);
    pos.x -= side_size / 2.0f;
    pos.y += side_size / 2.0f;
    return pos;
}

bool ui_match_point_to_board_cell(ui_info_t info, const board_t* b, quartz_vec2 point, int* x, int* y)
{
    float abstract_cell_size = info.cell_diplay_size + info.wall_diplay_size / 2.0f;

    quartz_vec2 origin = ui_calc_board_origin((quartz_vec2){0}, info, b);
    quartz_vec2 diff = { point.x - origin.x, point.y - origin.y };

    diff.x = floor(diff.x / abstract_cell_size);
    diff.y = floor(-diff.y / abstract_cell_size);

    if(diff.x >= 0 && diff.x < b->size && diff.y >= 0 && diff.y < b->size)
    {
        if(x != NULL) *x = diff.x;
        if(y != NULL) *y = diff.y;
        return true;
    }

    return false;
}

bool ui_match_board_cell_to_point(ui_info_t info, const board_t* b, int x, int y, quartz_vec2* point)
{
    if(x < 0 || x >= b->size || y < 0 || y >= b->size) return false;

    if(point != NULL)
    {
        quartz_vec2 origin = ui_calc_board_origin((quartz_vec2){0}, info, b);
        point->x = origin.x + info.cell_diplay_size/2.0f + x * (info.cell_diplay_size + info.wall_diplay_size);
        point->y = origin.y - info.cell_diplay_size/2.0f - y * (info.cell_diplay_size + info.wall_diplay_size);
    }

    return true;
}

const char* ui_get_winner_text(player_t winner)
{
    switch(winner)
    {
        case NO_PLAYER: return "Draw";
        case PLAYER_1: return "X Won";
        case PLAYER_2: return "O Won";
        default: assert(false);
    }
}

void ui_draw_board(ui_info_t info, const board_t* b)
{
    quartz_vec2 origin = ui_calc_board_origin((quartz_vec2){0}, info, b);

    for(int y = 0; y < b->size; y++)
    {
        for(int x = 0; x < b->size; x++)
        {
            quartz_vec2 pos = origin;
            pos.x += x * info.cell_diplay_size + info.cell_diplay_size / 2.0f;
            pos.y += -y * info.cell_diplay_size - info.cell_diplay_size / 2.0f;

            pos.x += info.wall_diplay_size * x;
            pos.y -= info.wall_diplay_size * y;

            quartz_render2D_quad(info.cell_color, pos, (quartz_vec2){info.cell_diplay_size, info.cell_diplay_size}, 0.0f);

            if(board_get_cell(b, x, y) == PLAYER_1)
                ui_draw_X(info.p1_color, pos, info.symbol_display_size);
            else if(board_get_cell(b, x, y) == PLAYER_2)
                ui_draw_O(info.p2_color, info.cell_color, pos, info.symbol_display_size);
        }
    }

    float side_size = ui_calc_board_side_size(info, b);

    for(int x = 1; x < b->size; x++)
    {
        quartz_vec2 pos = origin;
        pos.x += (x * info.cell_diplay_size) + ((x-1) * info.wall_diplay_size) + info.wall_diplay_size/2.0f;
        pos.y = 0;
        quartz_render2D_quad(info.wall_color, pos, (quartz_vec2){info.wall_diplay_size, side_size}, 0.0f);
    }

    for(int y = 1; y < b->size; y++)
    {
        quartz_vec2 pos = origin;
        pos.x = 0;
        pos.y -= (y * info.cell_diplay_size) + ((y-1) * info.wall_diplay_size) + info.wall_diplay_size/2.0f;
        quartz_render2D_quad(info.wall_color, pos, (quartz_vec2){side_size, info.wall_diplay_size}, 0.0f);
    }
}

void ui_draw_X(quartz_color color, quartz_vec2 pos, float size)
{
    quartz_render2D_quad(color, pos, (quartz_vec2){ size/4, size }, 3.1415/4.0f);
    quartz_render2D_quad(color, pos, (quartz_vec2){ size/4, size }, -3.1415/4.0f);
}

void ui_draw_O(quartz_color outer_color, quartz_color inner_color, quartz_vec2 pos, float size)
{
    quartz_render2D_circle(outer_color, pos, size/2);
    quartz_render2D_circle(inner_color, pos, size/3);
}

void ui_draw_text_centered(quartz_font font, float font_size, const char* text, quartz_vec2 pos, quartz_color color)
{
    quartz_vec2 text_size = quartz_font_get_text_size(font, font_size, text);
    pos.x -= text_size.x/2;
    pos.y += text_size.y/2;
    quartz_render2D_text(font, font_size, text, pos, color);
}

quartz_aabb2 ui_get_button_aabb(const ui_button_t* btn)
{
    quartz_aabb2 box = { .x = btn->position.x, .y = btn->position.y, .hwidth = btn->scale.x/2, .hheight = btn->scale.y/2 };
    return box;
}

bool ui_check_button_hover(ui_button_t* btn, quartz_vec2 point)
{
    if(btn->disabled) return btn->hovered = false;

    quartz_aabb2 box = ui_get_button_aabb(btn);
    return btn->hovered = quartz_aabb2_touches_point(point, box);
}

void ui_draw_button(const ui_button_t* btn, quartz_font font, float font_size, const char* text, quartz_color text_color, quartz_color base_color, quartz_color hover_color)
{
    if(btn->disabled) return;

    quartz_color color = btn->hovered ? hover_color : base_color;
    quartz_render2D_quad(color, btn->position, btn->scale, 0.0f);
    ui_draw_text_centered(font, font_size, text, btn->position, text_color);
}
