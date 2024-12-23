#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

#include "../external/quartz.h"

typedef struct scene_selector_t scene_selector_t;

typedef struct
{
    void* (*make)(const quartz_font* font, const quartz_camera2D* camera);
    void (*update)(scene_selector_t* selector, void* ctx);
    void (*free)(void* ctx);
} scene_t;

struct scene_selector_t
{
    const quartz_font* font;
    const quartz_camera2D* camera;

    bool should_change_scene;
    scene_t next_scene;

    void* curr_ctx;
    scene_t curr_scene;
};

scene_selector_t scene_selector_make(const quartz_font* font, const quartz_camera2D* camera, scene_t start_scene);
void scene_selector_check_for_new_scene(scene_selector_t* selector);
void scene_selector_change(scene_selector_t* selector, scene_t new_scene);
void scene_selector_update(scene_selector_t* selector);
void scene_selector_free(scene_selector_t* selector);

#endif