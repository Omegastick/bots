#pragma once

#include <string>

#include "asset_store.h"
#include "graphics/texture.h"
#include "graphics/shader.h"

namespace SingularityTrainer
{
class ResourceManager
{
  public:
    ResourceManager(std::string path_to_assets_folder);
    ~ResourceManager();

    void load_texture(const std::string &id, const std::string &path);
    void load_shader(const std::string &id, const std::string &vert_path, const std::string &frag_path);

    AssetStore<Texture> texture_store;
    AssetStore<Shader> shader_store;

  private:
    std::string base_path;
};
}