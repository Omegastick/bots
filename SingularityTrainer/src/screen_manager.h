#pragma once

#include <stack>

#include "iscreen.h"
#include "graphics/renderers/renderer.h"

namespace SingularityTrainer
{
class ScreenManager
{
  public:
    void show_screen(std::shared_ptr<IScreen> screen);
    void close_screen();
    void update(float delta_time);
    int stack_size();
    void draw(Renderer &renderer, bool lightweight = false);

  private:
    std::stack<std::shared_ptr<IScreen>> screens;
};
}