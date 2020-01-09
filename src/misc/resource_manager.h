#pragma once

#include <string>

#include "audio/audio_source.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/font.h"
#include "misc/asset_store.h"
#include "third_party/di.hpp"

namespace ai
{
class Texture;
class Shader;
class Font;

static auto AssetsPath = [] {};

class ResourceManager
{
  public:
    BOOST_DI_INJECT(ResourceManager, (named = AssetsPath) std::string path_to_assets_folder);

    void load_audio_source(const std::string &id, const std::string &path);
    void load_font(const std::string &id, const std::string &path, float size);
    void load_shader(const std::string &id,
                     const std::string &vert_path,
                     const std::string &frag_path);
    void load_texture(const std::string &id, const std::string &path);

    AssetStore<AudioSource> audio_source_store;
    AssetStore<Font> font_store;
    AssetStore<Shader> shader_store;
    AssetStore<Texture> texture_store;

  private:
    std::string base_path;
};
}