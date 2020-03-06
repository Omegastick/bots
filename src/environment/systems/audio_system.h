#pragma once

#include <entt/entity/registry.hpp>

namespace ai
{
class IAudioEngine;

void audio_system(entt::registry &registry, IAudioEngine &audio_engine);
}