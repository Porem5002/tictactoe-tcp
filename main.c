#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "ai.h"
#include "ui.h"

#include "external/quartz.h"

int main(void)
{
    quartz_start(800, 600, "TicTacToe");

    ui_info_t ui_info = {
        .background_color = QUARTZ_BLACK,
        .cell_color = { 0.95, 0.95, 0.95, 1.0 },
        .p1_color = QUARTZ_RED,
        .p2_color = QUARTZ_BLUE,
        .cell_diplay_size = 100,
        .symbol_display_size = 50
    };

    game_t game = game_make(3, PLAYER_1);

    bool is_ai [] = {
        [PLAYER_1] = true,
        [PLAYER_2] = false,
    };

    quartz_camera2D camera = quartz_init_camera2D(800, 600);

    quartz_render2D_init();
    quartz_render2D_set_camera(&camera);

    while(quartz_update())
    {
        quartz_ivec2 mouse_screen_pos = quartz_get_mouse_pos();
        quartz_vec2 mouse_pos = quartz_camera2D_to_world_through_viewport(&camera, mouse_screen_pos, quartz_get_screen_viewport());

        if(!game.running && quartz_is_key_down(QUARTZ_KEY_SPACE))
            game_restart(&game);
        else
        {
            int x, y;

            if(is_ai[game.curr_player])
            {
                ai_next_move(&game.board, &x, &y, game.curr_player);
                game_do_move(&game, x, y);
            }
            else if(quartz_is_key_down(QUARTZ_KEY_L_MOUSE_BTN))
            {
                if(ui_match_point_to_board_cell(ui_info, &game.board, mouse_pos, &x, &y))
                    game_do_move(&game, x, y);
            }
        }

        quartz_clear(ui_info.background_color);
        ui_draw_board(ui_info, &game.board);
        quartz_render2D_flush();
    }

    game_free(&game);
    quartz_finish();
    return 0;
}
