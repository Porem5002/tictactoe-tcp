#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

#include <quartz/quartz.h>

typedef struct scene_selector_t scene_selector_t;

typedef struct
{
    quartz_viewport viewport;
    quartz_font font;
    const quartz_camera2D* camera;

    quartz_texture arrow_texture;
} scene_persistent_data_t;

typedef struct
{
    void* (*make)(scene_persistent_data_t pdata);
    void (*update)(scene_selector_t* selector, void* ctx);
    void (*free)(void* ctx);
} scene_t;

struct scene_selector_t
{
    scene_persistent_data_t pdata;

    bool should_change_scene;
    scene_t next_scene;

    void* curr_ctx;
    scene_t curr_scene;
};

scene_selector_t scene_selector_make(scene_persistent_data_t pdata, scene_t start_scene);
void scene_selector_check_for_new_scene(scene_selector_t* selector);
void scene_selector_change(scene_selector_t* selector, scene_t new_scene);
void scene_selector_update(scene_selector_t* selector);
void scene_selector_free(scene_selector_t* selector);

#endif