#pragma once

#include <unordered_map>

#include "graphics/backend/texture.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
class IModuleFactory;

class ModuleTextureStore
{
    std::unordered_map<std::string, Texture> cache;
    IModuleFactory &module_factory;
    Renderer renderer;

  public:
    ModuleTextureStore(IModuleFactory &module_factory, Renderer &&renderer);

    Texture &get(const std::string &module);
};
}