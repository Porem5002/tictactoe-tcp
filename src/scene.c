#include <assert.h>

#include "scene.h"

scene_selector_t scene_selector_make(const quartz_font* font, const quartz_camera2D* camera, scene_t start_scene)
{
    assert(start_scene.make != NULL);
    
    scene_selector_t selector = {0};
    selector.font = font;
    selector.camera = camera;
    selector.curr_ctx = start_scene.make(font, camera);
    selector.curr_scene = start_scene;
    return selector;
}

void scene_selector_check_for_new_scene(scene_selector_t* selector)
{
    if(selector->should_change_scene)
    {
        selector->curr_scene.free(selector->curr_ctx);
    
        void* new_ctx = selector->next_scene.make(selector->font, selector->camera);
        selector->curr_ctx = new_ctx;    
        selector->curr_scene = selector->next_scene;

        selector->should_change_scene = false;
        memset(&selector->next_scene, 0, sizeof(selector->next_scene));
    }
}

void scene_selector_change(scene_selector_t* selector, scene_t new_scene)
{
    selector->should_change_scene = true;
    selector->next_scene = new_scene;
}

void scene_selector_update(scene_selector_t* selector)
{
    assert(selector->curr_scene.update != NULL);
    selector->curr_scene.update(selector, selector->curr_ctx);
}

void scene_selector_free(scene_selector_t* selector)
{
    assert(selector->curr_scene.free != NULL);
    selector->curr_scene.free(selector->curr_ctx);
    memset(selector, 0, sizeof(scene_selector_t));
}
