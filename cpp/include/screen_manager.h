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

    void show_screen(std::shared_ptr<IScreen> screen);
    void close_screen();
    void update(sf::Time delta_time);
    void draw(sf::RenderTarget &render_target);

  private:
    std::stack<std::shared_ptr<IScreen>> screens_;
};
}
