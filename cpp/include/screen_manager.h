#pragma once

#include <SFML/Graphics.hpp>
#include <stack>

#include "iscreen.h"

namespace STrainer
{
class ScreenManager
{
  public:
    static ScreenManager &get_instance()
    {
        static ScreenManager instance;
        return instance;
    }

    void show_screen(IScreen *screen);
    void close_screen();
    void update(float delta_time);
    void draw(sf::RenderTarget &render_target);

    ScreenManager(const ScreenManager &) = delete;
    void operator=(const ScreenManager &) = delete;

  private:
    std::stack<IScreen *> screens_;

    ScreenManager() {}
};
}
