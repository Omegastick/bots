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

class CrtTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    std::unique_ptr<Sprite> sprite;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;
    std::unique_ptr<PostProcLayer> post_proc_layer;

  public:
    CrtTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~CrtTestScreen();

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}