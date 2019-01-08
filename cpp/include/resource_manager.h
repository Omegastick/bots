#pragma once

#include <SFML/Graphics.hpp>

#include "asset_store.h"

namespace SingularityTrainer
{
class ResourceManager
{
  public:
    ResourceManager() {}
    ~ResourceManager() {}

    void load_texture(const std::string &id, const std::string &path);

    AssetStore<sf::Texture> texture_store;
};
}