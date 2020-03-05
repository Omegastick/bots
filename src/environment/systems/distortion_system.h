#pragma once

#include <entt/entity/registry.hpp>

#include "graphics/renderers/renderer.h"

namespace ai
{
void distortion_system(entt::registry &registry, Renderer &renderer);
}