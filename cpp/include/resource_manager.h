#pragma once

#include <string>
#include <SFML/Graphics.hpp>

#include "asset_store.h"

namespace SingularityTrainer
{
class ResourceManager
{
  public:
    ResourceManager(std::string path_to_assets_folder);
    ~ResourceManager();

    void load_texture(const std::string &id, const std::string &path);
    void load_font(const std::string &id, const std::string &path);

    AssetStore<sf::Texture> texture_store;
    AssetStore<sf::Font> font_store;

  private:
    std::string base_path;
};
}