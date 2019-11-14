#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <nanovg.h>

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class Sprite;
class Renderer;
class ResourceManager;
class ScreenManager;

class VectorTestScreen : public IScreen
{
  private:
    ResourceManager &resource_manager;
    std::vector<std::shared_ptr<IScreen>> &screens;
    std::vector<std::string> &screen_names;
    ScreenManager &screen_manager;
    glm::mat4 projection;
    double rotation;
    NVGcontext *vg;

  public:
    VectorTestScreen(ScreenManager &screen_manager,
                     ResourceManager &resource_manager,
                     std::vector<std::shared_ptr<IScreen>> &screens,
                     std::vector<std::string> &screen_names);
    ~VectorTestScreen();

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}