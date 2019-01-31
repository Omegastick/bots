#include <SFML/Graphics.hpp>

#include "resource_manager.h"

namespace SingularityTrainer
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

    std::shared_ptr<sf::Texture> texture = std::make_shared<sf::Texture>();

    if (!texture->loadFromFile(full_path))
    {
        return;
    }

    texture_store.add(id, texture);
};

void ResourceManager::load_font(const std::string &id, const std::string &path)
{
    std::string full_path = base_path + path;

    if (font_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<sf::Font> font = std::make_shared<sf::Font>();

    if (!font->loadFromFile(full_path))
    {
        return;
    }

    font_store.add(id, font);
};

void ResourceManager::load_shader(const std::string &id, const std::string &path)
{
    std::string full_path = base_path + path;

    if (shader_store.check_exists(id))
    {
        return;
    }

    std::shared_ptr<sf::Shader> shader = std::make_shared<sf::Shader>();

    if (!shader->loadFromFile(full_path, sf::Shader::Fragment))
    {
        return;
    }

    shader_store.add(id, shader);
};
}