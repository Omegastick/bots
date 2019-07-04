#pragma once

#include <string>

#include "misc/asset_store.h"
#include "third_party/di.hpp"

namespace SingularityTrainer
{
class Texture;
class Shader;
class Font;

static auto AssetsPath = [] {};

class ResourceManager
{
  public:
    BOOST_DI_INJECT(ResourceManager, (named = AssetsPath) std::string path_to_assets_folder);
    ~ResourceManager();

    void load_texture(const std::string &id, const std::string &path);
    void load_shader(const std::string &id, const std::string &vert_path, const std::string &frag_path);
    void load_font(const std::string &id, const std::string &path, float size);

    AssetStore<Texture> texture_store;
    AssetStore<Shader> shader_store;
    AssetStore<Font> font_store;

  private:
    std::string base_path;
};
}