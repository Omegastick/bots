#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class Renderer;
class ResourceManager;
class ScreenManager;
class Line;

class LineTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;
    std::vector<Line> lines;

  public:
    LineTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~LineTestScreen();

    virtual void update(const double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}