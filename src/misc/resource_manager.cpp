#include <memory>
#include <string>

#include <soloud_wav.h>
#include <spdlog/spdlog.h>

#include "audio/audio_source.h"
#include "misc/resource_manager.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/shader.h"
#include "graphics/font.h"

namespace ai
{
ResourceManager::ResourceManager(std::string path_to_assets_folder)
    : base_path(path_to_assets_folder) {}

void ResourceManager::load_audio_source(const std::string &id, const std::string &path)
{
    const std::string full_path = base_path + path;
    if (audio_source_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<SoLoud::Wav> audio_source = std::make_shared<SoLoud::Wav>();
    audio_source->load(full_path.c_str());

    audio_source_store.add(id, audio_source);
}

void ResourceManager::load_font(const std::string &id, const std::string &path, float size)
{
    const std::string full_path = base_path + path;
    if (font_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<Font> font = std::make_shared<Font>(full_path, size);

    font_store.add(id, font);
}

void ResourceManager::load_shader(const std::string &id,
                                  const std::string &vert_path,
                                  const std::string &frag_path)
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

void ResourceManager::load_texture(const std::string &id, const std::string &path)
{
    const std::string full_path = base_path + path;
    if (texture_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<Texture> texture = std::make_shared<Texture>(full_path);

    texture_store.add(id, texture);
}

void ResourceManager::unload_all()
{
    audio_source_store.unload();
    font_store.unload();
    shader_store.unload();
    texture_store.unload();
}
}