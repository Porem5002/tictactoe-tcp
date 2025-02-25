#ifndef ANIM_H
#define ANIM_H

#include <stdbool.h>
#include <stdlib.h>

#include <quartz/quartz.h>

typedef enum
{
    ANIM_STATUS_CONTINUE,
    ANIM_STATUS_FINISHED,
} anim_status_t;

typedef struct
{
    float t;
    float duration;
    float (*ease)(float t);
} anim_t;

float anim_get_value(const anim_t* anim);
anim_status_t anim_update(anim_t* anim, float inc);
void anim_reset(anim_t* anim);

typedef struct
{
    anim_t anim;
    float factor;
    float base;
    float* target;
} anim_property_t;

float anim_property_get_value(const anim_property_t* prop);
anim_status_t anim_property_update(anim_property_t* prop, float inc);
void anim_property_reset(anim_property_t* prop);

typedef struct
{
    size_t cap;
    size_t offset;
    size_t base;
    anim_property_t* buffer;
} anim_writer_t;

void anim_writer_write_float(anim_writer_t* wr, anim_t anim, float start, float end, float* target);
void anim_writer_write_vec2(anim_writer_t* wr, anim_t anim,  quartz_vec2 start, quartz_vec2 end, quartz_vec2* target);
void anim_writer_write_color3(anim_writer_t* wr, anim_t anim,  quartz_color start, quartz_color end, quartz_color* target);
void anim_writer_write_color4(anim_writer_t* wr, anim_t anim,  quartz_color start, quartz_color end, quartz_color* target);

void anim_writer_rebase(anim_writer_t* wr);
size_t anim_writer_get_size(const anim_writer_t* wr);
anim_property_t* anim_writer_get_baseptr(const anim_writer_t* wr);

float anim_sin01(float t);

#endif
