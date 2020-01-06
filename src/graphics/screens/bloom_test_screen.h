#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "screens/iscreen.h"

namespace ai
{
struct Sprite;
class Renderer;
class PostProcLayer;
class ResourceManager;
class ScreenManager;

class BloomTestScreen : public IScreen
{
  private:
    ResourceManager &resource_manager;
    std::vector<std::shared_ptr<IScreen>> &screens;
    std::vector<std::string> &screen_names;
    std::unique_ptr<Sprite> sprite;
    ScreenManager &screen_manager;
    glm::mat4 projection;
    std::unique_ptr<PostProcLayer> post_proc_layer;

  public:
    BloomTestScreen(ScreenManager &screen_manager,
                    ResourceManager &resource_manager,
                    std::vector<std::shared_ptr<IScreen>> &screens,
                    std::vector<std::string> &screen_names);

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}