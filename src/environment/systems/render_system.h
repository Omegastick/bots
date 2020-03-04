#pragma once

#include <entt/fwd.hpp>

namespace ai
{
class Renderer;

void debug_render_system(entt::registry &registry, Renderer &renderer);
void render_system(entt::registry &registry, Renderer &renderer);
}