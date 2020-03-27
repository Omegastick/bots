#include <unordered_map>

#include <doctest.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "module_texture_store.h"
#include "environment/components/module_link.h"
#include "environment/systems/physics_system.h"
#include "environment/systems/render_system.h"
#include "environment/utils/body_factories.h"
#include "graphics/backend/texture.h"
#include "graphics/renderers/renderer.h"
#include "misc/module_factory.h"
#include "training/modules/imodule.h"

namespace ai
{
constexpr int image_size = 500;

const glm::mat4 projection = glm::ortho(-1, 1, -1, 1);

ModuleTextureStore::ModuleTextureStore(Renderer &&renderer)
    : renderer(std::move(renderer))
{
    init_physics(registry);
}

Texture &ModuleTextureStore::get(const std::string &module)
{
    if (!renderer.is_initialized())
    {
        renderer.init();
    }

    const auto found_texture = cache.find(module);
    if (found_texture != cache.end())
    {
        return found_texture->second;
    }

    spdlog::debug("Creating module texture for {}", module);
    renderer.resize(image_size, image_size);
    renderer.begin();
    renderer.set_view(projection);

    make_module(registry, module);
    for (const auto &entity : registry.view<EcsModuleLink>())
    {
        registry.destroy(entity);
    }
    render_system(registry, renderer);

    const auto *frame_buffer = renderer.render_to_buffer(0);

    cache.insert(std::make_pair(module, Texture(image_size, image_size, nullptr, GL_RGBA)));
    cache.find(module)->second.bind();
    frame_buffer->bind_read();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 0, 0, image_size, image_size, 0);

    registry.clear();

    return cache.find(module)->second;
}
}