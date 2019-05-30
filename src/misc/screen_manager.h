#pragma once

#include <memory>
#include <stack>

namespace SingularityTrainer
{
class IScreen;
class Renderer;

class ScreenManager
{
  public:
    ScreenManager();

    void show_screen(std::shared_ptr<IScreen> screen);
    void close_screen();
    void update(double delta_time);
    int stack_size();
    void draw(Renderer &renderer, bool lightweight = false);

  private:
    std::stack<std::shared_ptr<IScreen>> screens;
    int screens_to_pop;
};
}
