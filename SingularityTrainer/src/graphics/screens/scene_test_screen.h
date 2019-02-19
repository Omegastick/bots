#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "graphics/renderers/renderer.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class SceneTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;

  public:
    SceneTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~SceneTestScreen();

    virtual void update(const float delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}