#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "base.h"
#include "ai.h"

#include "external/quartz.h"

#define CELL_DISPLAY_SIZE 100 

void draw_X(quartz_color color, quartz_vec2 pos, float size)
{
    quartz_render2D_quad(color, pos, (quartz_vec2){ size/4, size }, 3.1415/4.0f);
    quartz_render2D_quad(color, pos, (quartz_vec2){ size/4, size }, -3.1415/4.0f);
}

void draw_O(quartz_color color, quartz_color color_bg, quartz_vec2 pos, float size)
{
    quartz_render2D_circle(color, pos, size/2);
    quartz_render2D_circle(color_bg, pos, size/3);
}

int main(void)
{
    quartz_start(800, 600, "TicTacToe");

    board_t b = board_make(3);
    
    bool is_ai [] = {
        [PLAYER_1] = true,
        [PLAYER_2] = false,
    };

    player_t curr_player = PLAYER_1;
    bool game_running = true;

    quartz_camera2D camera = quartz_init_camera2D(800, 600);

    quartz_render2D_init();
    quartz_render2D_set_camera(&camera);

    while(quartz_update())
    {
        quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
        quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(&camera, mouse_screen_pos, quartz_get_screen_viewport());

        if(!game_running)
        {
            if(quartz_is_key_down(QUARTZ_KEY_SPACE))
            {
                game_running = true;
                curr_player = PLAYER_1;
                board_clear(&b);
            }
        }
        else if(board_is_final(&b, NULL))
            game_running = false;
        else
        {
            int x = -1, y = -1;

            if(!is_ai[curr_player])
            {
                if(quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
                {
                    quartz_vec2 map_origin = { -CELL_DISPLAY_SIZE/2.0f * b.size, CELL_DISPLAY_SIZE/2.0f * b.size };
                    quartz_vec2 diff = { mouse_pos.x - map_origin.x, mouse_pos.y - map_origin.y };

                    diff.x = floor(diff.x / CELL_DISPLAY_SIZE);
                    diff.y = floor(-diff.y / CELL_DISPLAY_SIZE);

                    if(diff.x >= 0 && diff.x < CELL_DISPLAY_SIZE && diff.y >= 0 && diff.y < CELL_DISPLAY_SIZE)
                    {
                        x = diff.x;
                        y = diff.y;
                    }
                }
            }
            else
                ai_next_move(&b, &x, &y, curr_player);
            

            if(x != -1 && y != -1 && board_get_cell(&b, x, y) == NO_PLAYER)
            {
                board_set_cell(&b, x, y, curr_player);
                curr_player = curr_player == PLAYER_1 ? PLAYER_2 : PLAYER_1;
            }
        }

        quartz_clear(QUARTZ_BLACK);

        for(int y = 0; y < b.size; y++)
        {
            for(int x = 0; x < b.size; x++)
            {
                quartz_vec2 pos = {0};
                pos.x += x * CELL_DISPLAY_SIZE - CELL_DISPLAY_SIZE * b.size / 2.0f + CELL_DISPLAY_SIZE / 2.0f;
                pos.y += -y * CELL_DISPLAY_SIZE + CELL_DISPLAY_SIZE * b.size / 2.0f - CELL_DISPLAY_SIZE / 2.0f;
                quartz_render2D_quad(QUARTZ_WHITE, pos, (quartz_vec2){100, 100}, 0.0f);

                if(board_get_cell(&b, x, y) == PLAYER_1)
                    draw_X(QUARTZ_RED, pos, 50);
                else if(board_get_cell(&b, x, y) == PLAYER_2)
                    draw_O(QUARTZ_BLUE, QUARTZ_WHITE, pos, 50);
            }
        }

        quartz_render2D_flush();
    }

    board_free(&b);
    quartz_finish();
    return 0;
}
