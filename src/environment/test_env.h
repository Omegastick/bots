#pragma once

#include <entt/entt.hpp>

namespace ai
{
class Renderer;

class TestEnv
{
  private:
    entt::registry registry;

  public:
    TestEnv();

    void draw(Renderer &renderer);
    void update(double delta_time);
};
}