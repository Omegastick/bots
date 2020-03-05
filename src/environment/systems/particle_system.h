#pragma once

#include <entt/entity/registry.hpp>

#include "graphics/renderers/renderer.h"

namespace ai
{
void particle_system(entt::registry &registry, Renderer &renderer);
}