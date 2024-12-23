#include <math.h>

#include "ui.h"

static void draw_X(quartz_color color, quartz_vec2 pos, float size);
static void draw_O(quartz_color outer_color, quartz_color inner_color, quartz_vec2 pos, float size);

bool ui_match_point_to_board_cell(ui_info_t info, const board_t* b, quartz_vec2 point, int* x, int* y)
{
    quartz_vec2 b_origin = { -info.cell_diplay_size/2.0f * b->size, info.cell_diplay_size/2.0f * b->size };
    quartz_vec2 diff = { point.x - b_origin.x, point.y - b_origin.y };

    diff.x = floor(diff.x / info.cell_diplay_size);
    diff.y = floor(-diff.y / info.cell_diplay_size);

    if(diff.x >= 0 && diff.x < b->size && diff.y >= 0 && diff.y < b->size)
    {
        if(x != NULL) *x = diff.x;
        if(y != NULL) *y = diff.y;
        return true;
    }

    return false;
}

void ui_draw_board(ui_info_t info, const board_t* b)
{
    for(int y = 0; y < b->size; y++)
    {
        for(int x = 0; x < b->size; x++)
        {
            quartz_vec2 pos = {0};
            pos.x += x * info.cell_diplay_size - info.cell_diplay_size * b->size / 2.0f + info.cell_diplay_size / 2.0f;
            pos.y += -y * info.cell_diplay_size + info.cell_diplay_size * b->size / 2.0f - info.cell_diplay_size / 2.0f;
            quartz_render2D_quad(info.cell_color, pos, (quartz_vec2){100, 100}, 0.0f);

            if(board_get_cell(b, x, y) == PLAYER_1)
                draw_X(info.p1_color, pos, info.symbol_display_size);
            else if(board_get_cell(b, x, y) == PLAYER_2)
                draw_O(info.p2_color, info.cell_color, pos, info.symbol_display_size);
        }
    }
}

static void draw_X(quartz_color color, quartz_vec2 pos, float size)
{
    quartz_render2D_quad(color, pos, (quartz_vec2){ size/4, size }, 3.1415/4.0f);
    quartz_render2D_quad(color, pos, (quartz_vec2){ size/4, size }, -3.1415/4.0f);
}

static void draw_O(quartz_color outer_color, quartz_color inner_color, quartz_vec2 pos, float size)
{
    quartz_render2D_circle(outer_color, pos, size/2);
    quartz_render2D_circle(inner_color, pos, size/3);
}
