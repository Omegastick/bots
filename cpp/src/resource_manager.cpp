#include <SFML/Graphics.hpp>

#include "resource_manager.h"

namespace SingularityTrainer
{
void ResourceManager::load_texture(const std::string &id, const std::string &path)
{
    if (texture_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<sf::Texture> texture = std::make_shared<sf::Texture>();

    if (!texture->loadFromFile(path))
    {
        return;
    }

    texture_store.add(id, texture);
};

void ResourceManager::load_font(const std::string &id, const std::string &path)
{
    if (font_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<sf::Font> font = std::make_shared<sf::Font>();

    if (!font->loadFromFile(path))
    {
        return;
    }

    font_store.add(id, font);
};
}