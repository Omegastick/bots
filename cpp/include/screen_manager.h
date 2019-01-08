#pragma once

#include <SFML/Graphics.hpp>
#include <stack>

#include "iscreen.h"

namespace SingularityTrainer
{
class ScreenManager
{
  public:
    ScreenManager() {}
    ~ScreenManager() {}

    void show_screen(IScreen *screen);
    void close_screen();
    void update(float delta_time);
    void draw(sf::RenderTarget &render_target);

  private:
    std::stack<IScreen *> screens_;
};
}
