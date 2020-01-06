#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include "misc/resource_manager.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/shader.h"
#include "graphics/font.h"

namespace ai
{
ResourceManager::ResourceManager(std::string path_to_assets_folder) : base_path(path_to_assets_folder) {}
ResourceManager::~ResourceManager() {}

void ResourceManager::load_texture(const std::string &id, const std::string &path)
{
    std::string full_path = base_path + path;

    if (texture_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<Texture> texture = std::make_shared<Texture>(full_path);

    texture_store.add(id, texture);
}

void ResourceManager::load_shader(const std::string &id, const std::string &vert_path, const std::string &frag_path)
{
    std::string full_vert_path = base_path + vert_path;
    std::string full_frag_path = base_path + frag_path;

    if (shader_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<Shader> shader = std::make_shared<Shader>(full_vert_path, full_frag_path);

    shader_store.add(id, shader);
}

void ResourceManager::load_font(const std::string &id, const std::string &path, float size)
{
    std::string full_path = base_path + path;

    if (font_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<Font> font = std::make_shared<Font>(full_path, size);

    font_store.add(id, font);
}
}