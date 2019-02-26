#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "graphics/backend/shader.h"
#include "graphics/sprite.h"
#include "graphics/renderers/renderer.h"
#include "resource_manager.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
class SpriteTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    std::unique_ptr<Sprite> sprite;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;

  public:
    SpriteTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~SpriteTestScreen();

    virtual void update(const float delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}