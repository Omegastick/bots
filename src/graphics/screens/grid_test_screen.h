#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/renderers/batched_sprite_renderer.h"
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

    std::vector<glm::vec3> accelerations;
    std::vector<glm::vec3> offsets;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;

    BatchedSpriteRenderer sprite_renderer;

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