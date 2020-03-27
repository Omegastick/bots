#pragma once

#include <unordered_map>

#include <entt/entity/registry.hpp>

#include "graphics/backend/texture.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
class ModuleTextureStore
{
    std::unordered_map<std::string, Texture> cache;
    entt::registry registry;
    Renderer renderer;

  public:
    ModuleTextureStore(Renderer &&renderer);

    Texture &get(const std::string &module);
};
}