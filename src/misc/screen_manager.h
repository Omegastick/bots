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

    void close_screen();
    void draw(Renderer &renderer, bool lightweight = false);
    void exit();
    void show_screen(std::shared_ptr<IScreen> screen);
    int stack_size();
    void update(double delta_time);

  private:
    std::stack<std::shared_ptr<IScreen>> screens;
    int screens_to_pop;
};
}
