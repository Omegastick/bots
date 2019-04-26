#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "random.h"

class b2World;

namespace SingularityTrainer
{
class Renderer;
class Agent;
class ScreenManager;
class ResourceManager;

class SceneTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;
    std::unique_ptr<Agent> agent;
    std::unique_ptr<b2World> b2_world;
    Random random;
    float elapsed_time;

  public:
    SceneTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~SceneTestScreen();

    virtual void update(const double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}