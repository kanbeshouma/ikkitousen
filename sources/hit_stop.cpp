#include "hit_stop.h"

void HitStop::update(GraphicsPipeline& graphics, float elapsed_time, CameraShake& camera_shake)
{
    if (!hit_stop) return;
    timer += elapsed_time;
    if (shake) camera_shake.shake(graphics, elapsed_time);
    if (timer >= hit_stop_time)
    {
        camera_shake.reset(graphics);
        timer = 0;
        hit_stop_time = 0;
        hit_stop = false;
        shake = false;
    }
}

void HitStop::damage_hit_stop()
{
    hit_stop_time = 0.1f;
    hit_stop = false;
    shake = true;
}