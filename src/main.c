#include <stdlib.h>

#include "scene.h"
#include "../external/quartz.h"

extern const scene_t menu_scene;

int main(void)
{
    quartz_start(800, 600, "TicTacToe");
    
    quartz_font font = quartz_load_font("assets/fonts/Lato-Black.ttf");
    quartz_camera2D camera = quartz_init_camera2D(800, 600);
    
    quartz_render2D_init();
    quartz_render2D_set_camera(&camera);

    scene_selector_t selector = scene_selector_make(&font, &camera, menu_scene);

    while(quartz_update())
    {
        scene_selector_check_for_new_scene(&selector);
        scene_selector_update(&selector);
    }

    scene_selector_free(&selector);
    quartz_finish();
    return 0;
}
