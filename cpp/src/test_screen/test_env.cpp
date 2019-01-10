#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>

#include "test_screen/bot.h"
#include "test_screen/test_env.h"
#include "test_screen/wall.h"

namespace SingularityTrainer
{
TestEnv::TestEnv(std::shared_ptr<ResourceManager> resource_manager, float x, float y, float scale)
{
    // Box2D world
    world = std::make_shared<b2World>(b2Vec2(0.f, 0.f));

    // Bot
    bot = std::make_unique<Bot>(resource_manager, *world);

    // Walls
    walls.push_back(Wall(-5.f, -5.f, 10.f, 0.1f, *world));
    walls.push_back(Wall(-5.f, -5.f, 0.1f, 10.f, *world));
    walls.push_back(Wall(-5.f, 4.9f, 10.f, 0.1f, *world));
    walls.push_back(Wall(4.9f, -5.f, 0.1f, 10.f, *world));

    // Display
    render_texture.create(1000, 1000);
    sprite.setTexture(render_texture.getTexture());
    sprite.setPosition(x, y);
    sprite.setScale(scale, scale);
}

TestEnv::~TestEnv(){};

TestEnv::TestEnv(TestEnv&& other) {
    other.sprite = std::move(sprite);
    other.world = std::move(world);
    other.bot = std::move(bot);
    other.walls = std::move(walls);
}

void TestEnv::draw(sf::RenderTarget &render_target)
{
    // Draw onto temporary texture
    render_texture.clear();
    bot->draw(render_texture);
    for (auto &wall : walls)
    {
        wall.draw(render_texture);
    }

    // Draw temporary tecture onto window
    sprite.setTexture(render_texture.getTexture());
}

std::unique_ptr<StepInfo> TestEnv::step(std::vector<bool> &actions)
{
    // world->Step(1.f / 60.f, 6, 4);
    return std::make_unique<StepInfo>();
}
}