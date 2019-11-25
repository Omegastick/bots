#include <unordered_map>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "module_texture_store.h"
#include "graphics/backend/texture.h"
#include "graphics/renderers/renderer.h"
#include "misc/module_factory.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
const glm::mat4 projection = glm::ortho(-1, 1, -1, 1);

ModuleTextureStore::ModuleTextureStore(ModuleFactory &module_factory, Renderer &&renderer)
    : module_factory(module_factory),
      renderer(std::move(renderer)) {}

const Texture &ModuleTextureStore::get(const std::string &module)
{
    const auto found_texture = cache.find(module);
    if (found_texture != cache.end())
    {
        return found_texture->second;
    }

    spdlog::debug("Creating module texture for {}", module);
    const auto constructed_module = module_factory.create_module(module);

    renderer.resize(300, 300);
    renderer.begin();
    renderer.set_view(projection);

    constructed_module->draw(renderer);

    const auto *frame_buffer = renderer.render_to_buffer(0);

    cache.insert(std::make_pair(module, Texture(300, 300, nullptr, GL_RGBA)));
    cache.find(module)->second.bind();
    frame_buffer->bind_read();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 0, 0, 300, 300, 0);

    return cache.find(module)->second;
}
}