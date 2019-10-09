#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class Sprite;
class ResourceManager;
class ScreenManager;
class Renderer;

class GridTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    std::unique_ptr<Sprite> sprite;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;

    std::vector<glm::vec2> accelerations;
    std::vector<glm::vec2> original_positions;
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> velocities;

  public:
    GridTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~GridTestScreen();

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}