#ifndef UI_H
#define UI_H

#include <stdbool.h>

#include "base.h"

#include "external/quartz.h"

typedef struct
{
    quartz_color background_color;
    quartz_color cell_color;

    quartz_color p1_color;
    quartz_color p2_color;

    float cell_diplay_size;
    float symbol_display_size;
} ui_info_t;

bool ui_match_point_to_board_cell(ui_info_t info, const board_t* b, quartz_vec2 point, int* x, int* y);
void ui_draw_board(ui_info_t info, const board_t* b);

#endif