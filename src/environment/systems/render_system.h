#pragma once

#include <entt/fwd.hpp>

namespace ai
{
class Renderer;

void render_system(entt::registry &registry, Renderer &renderer);
}