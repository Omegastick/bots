#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "idrawable.h"
#include "test_screen/bot.h"
#include "test_screen/target.h"
#include "test_screen/wall.h"

namespace SingularityTrainer
{
class Target;

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

    void draw(sf::RenderTarget &render_target);
    std::unique_ptr<StepInfo> step(std::vector<bool> &actions);
    void reset();
    void change_reward(float reward_delta);
    void set_done();
    
    float reward;

  private:
    sf::RenderTexture render_texture;
    sf::Sprite sprite;
    std::unique_ptr<b2World> world;
    std::unique_ptr<Bot> bot;
    std::unique_ptr<Target> target;
    std::vector<std::unique_ptr<Wall>> walls;
    std::unique_ptr<b2ContactListener> contact_listener;
    bool done;
};
}