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

void ui_text_draw_inline(quartz_font font, float font_size, const char* text, quartz_vec2 pos, quartz_color color)
{
    quartz_vec2 text_size = quartz_font_get_text_size(font, font_size, text);
    pos.x -= text_size.x/2;
    pos.y += text_size.y/2;
    quartz_render2D_text(font, font_size, text, pos, color);
}

quartz_aabb2 ui_button_get_aabb(const ui_button_t* btn)
{
    quartz_aabb2 aabb = {
        .x = btn->position.x,
        .y = btn->position.y,
        .hwidth = btn->scale.x/2.0f,
        .hheight = btn->scale.y/2.0f
    };
    return aabb;
}

bool ui_button_update(ui_button_t* btn, quartz_vec2 point)
{
    quartz_aabb2 aabb = ui_button_get_aabb(btn);
    btn->selected = !btn->disabled && quartz_aabb2_touches_point(point, aabb);

    float dt = quartz_get_delta_time();
    if(!btn->selected) dt *= -1;

    for(size_t i = 0; i < btn->anims_size; i++)
        anim_property_update(&btn->anims[i], dt);

    return btn->selected;
}

void ui_button_draw(const ui_button_t* btn)
{
    if(!btn->disabled)
        quartz_render2D_quad(btn->color, btn->position, btn->scale, 0.0f);
}

void ui_text_draw(const ui_text_t* text)
{
    ui_text_draw_inline(text->font, text->font_size, text->text, text->position, text->color);
}

void ui_texture_draw(const ui_texture_t* texture)
{
    quartz_vec2 back_text_scale = { 
        texture->scale.x/quartz_texture_get_info(texture->texture).width,
        texture->scale.y/quartz_texture_get_info(texture->texture).height
    };
    quartz_render2D_texture(texture->texture, texture->position, back_text_scale, texture->rotation, texture->tint);
}

anim_property_t ui_get_player_my_turn_anim()
{
    return (anim_property_t){ .anim = { .duration = 0.65, .ease = anim_sin01 }, .base = 90, .factor = 20 };
}

void ui_fill_texture_button_anims(anim_writer_t* wr, ui_button_t* btn, ui_texture_t* texture)
{
    assert(wr != NULL);
    assert(btn != NULL);
    assert(texture != NULL);

    anim_t base_anim = { .duration = 0.15 };
    float btn_scale_increase = 10;
    float lighten_factor = 0.30;
    float texture_scale_increase = 5;
    
    quartz_vec2 btn_end_scale = { btn->scale.x + btn_scale_increase, btn->scale.y + btn_scale_increase };
    quartz_color btn_end_color = ui_ligthen_color(btn->color, lighten_factor);
    quartz_vec2 texture_end_scale = { texture->scale.x + texture_scale_increase, texture->scale.y + texture_scale_increase };

    anim_writer_rebase(wr);

    anim_writer_write_vec2(wr, base_anim, btn->scale, btn_end_scale, &btn->scale);
    anim_writer_write_color3(wr, base_anim, btn->color, btn_end_color, &btn->color);
    anim_writer_write_vec2(wr, base_anim, texture->scale, texture_end_scale, &texture->scale);

    btn->anims_size = anim_writer_get_size(wr);
    btn->anims = anim_writer_get_baseptr(wr);
}

void ui_fill_text_button_anims(anim_writer_t* wr, ui_button_t* btn, ui_text_t* text)
{
    assert(wr != NULL);
    assert(btn != NULL);
    assert(text != NULL);

    anim_t base_anim = { .duration = 0.15 };
    float btn_scale_increase = 10;
    float lighten_factor = 0.30;
    float font_size_increase = 3;

    quartz_vec2 btn_end_scale = (quartz_vec2){btn->scale.x + btn_scale_increase, btn->scale.y + btn_scale_increase};
    quartz_color btn_end_color = ui_ligthen_color(btn->color, lighten_factor);
    float text_end_font_size = text->font_size + font_size_increase;

    anim_writer_rebase(wr);

    anim_writer_write_vec2(wr, base_anim, btn->scale, btn_end_scale, &btn->scale);
    anim_writer_write_color3(wr, base_anim, btn->color, btn_end_color, &btn->color);
    anim_writer_write_float(wr, base_anim, text->font_size, text_end_font_size, &text->font_size);

    btn->anims_size = anim_writer_get_size(wr);
    btn->anims = anim_writer_get_baseptr(wr);
}