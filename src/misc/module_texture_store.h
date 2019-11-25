#pragma once

#include <unordered_map>

#include "graphics/backend/texture.h"
#include "graphics/renderers/renderer.h"

namespace SingularityTrainer
{
class ModuleFactory;

class ModuleTextureStore
{
    std::unordered_map<std::string, Texture> cache;
    ModuleFactory &module_factory;
    Renderer renderer;

  public:
    ModuleTextureStore(ModuleFactory &module_factory, Renderer &&renderer);

    const Texture &get(const std::string &module);
};
}