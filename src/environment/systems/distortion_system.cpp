#pragma once

#include <entt/entt.hpp>

#include "distortion_system.h"
#include "environment/components/distortion_emitter.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
void distortion_system(entt::registry &registry, Renderer &renderer)
{
    registry.view<DistortionEmitter>().each([&](auto entity, auto &emitter) {
        if (emitter.explosive)
        {
            renderer.apply_explosive_force(emitter.position, emitter.size, emitter.strength);
        }
        else
        {
            renderer.apply_implosive_force(emitter.position, emitter.size, emitter.strength);
        }

        if (!emitter.loop)
        {
            registry.destroy(entity);
        }
    });
}
}