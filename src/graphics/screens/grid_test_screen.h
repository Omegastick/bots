#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/renderers/batched_sprite_renderer.h"
#include "misc/spring_mesh.h"
#include "screens/iscreen.h"

namespace ai
{
struct Sprite;
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

    SpringMesh spring_mesh;

    BatchedSpriteRenderer sprite_renderer;

  public:
    GridTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}