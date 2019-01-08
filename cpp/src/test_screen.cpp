#include <SFML/Graphics.hpp>

#include "test_screen.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
TestScreen::TestScreen(sf::RenderTarget &window, ResourceManager &resourceManager)
{
    resourceManager.load_texture("arrow", "cpp/assets/images/Arrow.png");
    std::shared_ptr<sf::Texture> texture = resourceManager.texture_store.get("arrow");
    arrow = sf::Sprite(*texture);
    arrow.setRotation(45.0f);
    arrow.setOrigin(arrow.getLocalBounds().width / 2.0f, arrow.getLocalBounds().height / 2.0f);
    arrow.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
}
TestScreen::~TestScreen() {}

void TestScreen::draw(sf::RenderTarget &render_target)
{
    render_target.draw(arrow);
}
void TestScreen::update(float delta_time)
{
    arrow.rotate(delta_time * 5);
}
}