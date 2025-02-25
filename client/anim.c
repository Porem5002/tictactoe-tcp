#include <stdlib.h>
#include <assert.h>

#include "anim.h"
#include "math.h"

float anim_get_value(const anim_t* anim)
{
    return anim->ease == NULL ? anim->t : anim->ease(anim->t);
}

anim_status_t anim_update(anim_t* anim, float inc)
{
    anim->t += inc / anim->duration;
    
    if(anim->t > 1.0f)
    {
        anim->t = 1.0f;
        return ANIM_STATUS_FINISHED;
    }
    
    if(anim->t < 0.0f)
    {
        anim->t = 0.0f;
        return ANIM_STATUS_FINISHED;
    }

    return ANIM_STATUS_CONTINUE;
}

void anim_reset(anim_t* anim)
{
    anim->t = 0.0f;
}

float anim_property_get_value(const anim_property_t* prop)
{
    return prop->base + prop->factor * anim_get_value(&prop->anim);
}

anim_status_t anim_property_update(anim_property_t* prop, float inc)
{
    anim_status_t status = anim_update(&prop->anim, inc);
    if(prop->target != NULL) *prop->target = anim_property_get_value(prop);
    return status;
}

void anim_property_reset(anim_property_t* prop)
{
    anim_reset(&prop->anim);
}

void anim_writer_write_float(anim_writer_t* wr, anim_t anim, float start, float end, float* target)
{
    assert(wr->base + wr->offset < wr->cap);

    anim_property_t prop = (anim_property_t){
        .anim = anim,
        .base = start,
        .factor = end - start,
        .target = target,
    };

    wr->buffer[wr->base + wr->offset] = prop;
    wr->offset++;
}

void anim_writer_write_vec2(anim_writer_t* wr, anim_t anim, quartz_vec2 start, quartz_vec2 end, quartz_vec2* target)
{
    bool is_target_null = target == NULL;
    anim_writer_write_float(wr, anim, start.x, end.x, is_target_null ? NULL : &target->x);
    anim_writer_write_float(wr, anim, start.y, end.y, is_target_null ? NULL : &target->y);
}

void anim_writer_write_color3(anim_writer_t* wr, anim_t anim, quartz_color start, quartz_color end, quartz_color* target)
{
    bool is_target_null = target == NULL;
    anim_writer_write_float(wr, anim, start.r, end.r, is_target_null ? NULL : &target->r);
    anim_writer_write_float(wr, anim, start.g, end.g, is_target_null ? NULL : &target->g);
    anim_writer_write_float(wr, anim, start.b, end.b, is_target_null ? NULL : &target->b);
}

void anim_writer_write_color4(anim_writer_t* wr, anim_t anim, quartz_color start, quartz_color end, quartz_color* target)
{
    bool is_target_null = target == NULL;
    anim_writer_write_float(wr, anim, start.r, end.r, is_target_null ? NULL : &target->r);
    anim_writer_write_float(wr, anim, start.g, end.g, is_target_null ? NULL : &target->g);
    anim_writer_write_float(wr, anim, start.b, end.b, is_target_null ? NULL : &target->b);
    anim_writer_write_float(wr, anim, start.a, end.a, is_target_null ? NULL : &target->a);
}

void anim_writer_rebase(anim_writer_t* wr)
{
    wr->base += wr->offset;
    wr->offset = 0;
}

size_t anim_writer_get_size(const anim_writer_t* wr)
{
    return wr->offset;
}

anim_property_t* anim_writer_get_baseptr(const anim_writer_t* wr)
{
    return wr->buffer + wr->base;
}

float anim_sin01(float t)
{
    return (sinf(t * 3.1415) + 1) / 2.0f;
}
