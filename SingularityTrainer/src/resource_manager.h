#pragma once

#include <string>

#include "asset_store.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/shader.h"
#include "graphics/font.h"

namespace SingularityTrainer
{
class ResourceManager
{
  public:
    ResourceManager(std::string path_to_assets_folder);
    ~ResourceManager();

    void load_texture(const std::string &id, const std::string &path);
    void load_shader(const std::string &id, const std::string &vert_path, const std::string &frag_path);
    void load_font(const std::string &id, const std::string &path);

    AssetStore<Texture> texture_store;
    AssetStore<Shader> shader_store;
    AssetStore<Font> font_store;

  private:
    std::string base_path;
};
}