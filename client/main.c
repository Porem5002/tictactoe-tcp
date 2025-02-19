#include <stdlib.h>

#include "scene.h"

#include "../shared/net.h"

extern const scene_t menu_scene;

int main(void)
{
    net_start();
    quartz_start(800, 600, "TicTacToe");
    
    quartz_viewport screen_vp = quartz_get_screen_viewport();
    quartz_viewport game_vp = quartz_make_viewport();

    quartz_font font = quartz_load_font("assets/fonts/Lato-Black.ttf");
    quartz_camera2D camera = quartz_init_camera2D(800, 600);
    
    quartz_render2D_init();
    quartz_render2D_set_viewport(game_vp);
    quartz_render2D_set_camera(&camera);

    scene_persistent_data_t pdata = { game_vp, font, &camera, quartz_load_texture("assets/textures/arrow.png") };
    scene_selector_t selector = scene_selector_make(pdata, menu_scene);

    while(quartz_update())
    {
        if(quartz_is_startup() || quartz_was_screen_resized())
        {
            quartz_rect new_rect = quartz_rect_calc_boxed(quartz_viewport_get_rect(screen_vp), 800.0f / 600.0f);
            quartz_viewport_set_rect(game_vp, new_rect);
            quartz_render2D_set_camera(&camera);
        }

        scene_selector_check_for_new_scene(&selector);
        scene_selector_update(&selector);
    }

    scene_selector_free(&selector);
    quartz_finish();
    net_finish();
    return 0;
}
