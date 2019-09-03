#pragma once

#include <deque>
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
    std::shared_ptr<IScreen> current_screen();
    void draw(Renderer &renderer, bool lightweight = false);
    void exit();
    void show_screen(std::shared_ptr<IScreen> screen);
    int stack_size();
    void update(double delta_time);

  private:
    enum class CommandType
    {
        Push,
        Pop
    };
    struct Command
    {
        CommandType type;
        std::shared_ptr<IScreen> screen = {};
    };

    std::stack<std::shared_ptr<IScreen>> screens;
    std::deque<Command> command_queue;
};
}
