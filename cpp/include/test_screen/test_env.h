#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "idrawable.h"
#include "test_screen/bot.h"
#include "test_screen/wall.h"

namespace SingularityTrainer
{
struct StepInfo
{
    std::vector<float> observation;
    float reward;
    bool done;
};

class TestEnv : IDrawable
{
  public:
    TestEnv(std::shared_ptr<ResourceManager> resource_manager, float x, float y, float scale);
    ~TestEnv();
    TestEnv(TestEnv&& other);

    void draw(sf::RenderTarget &render_target);
    std::unique_ptr<StepInfo> step(std::vector<bool> &actions);
    void reset();

  private:
    sf::RenderTexture render_texture;
    sf::Sprite sprite;
    std::unique_ptr<b2World> world;
    std::unique_ptr<Bot> bot;
    std::vector<Wall> walls;
};
}