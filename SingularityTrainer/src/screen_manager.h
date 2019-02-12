#pragma once

#include <SFML/Graphics.hpp>
#include <stack>

#include "iscreen.h"
#include "graphics/renderer.h"

namespace SingularityTrainer
{
class ScreenManager
{
  public:
    ScreenManager() {}
    ~ScreenManager() {}

    void show_screen(std::shared_ptr<IScreen> screen);
    void close_screen();
    void update(float delta_time);
    int stack_size();
    void draw(float delta_time, Renderer &renderer, bool lightweight = false);

  private:
    std::stack<std::shared_ptr<IScreen>> screens;
};
}
