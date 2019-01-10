#pragma once

#include <vector>
#include <memory>
#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "test_screen/bot.h"
#include "test_screen/wall.h"

namespace SingularityTrainer
{
class TestEnv : IDrawable
{
  public:
    TestEnv(float x, float y);
    ~TestEnv();

    void draw(sf::RenderTarget &render_target);
    std::unique_ptr<StepInfo> step(std::vector<bool> actions);
    void reset();

  private:
    sf::RenderTexture render_texture;
    b2World world;
    std::unique_ptr<Bot> bot;
    std::vector<std::unique_ptr<Wall>> walls;
};

struct StepInfo {
    std::vector<float> observation;
    float reward;
    bool done;
};
}