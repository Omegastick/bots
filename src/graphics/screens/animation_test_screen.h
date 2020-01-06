#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "screens/iscreen.h"

namespace ai
{
class Animator;
struct Sprite;
class ResourceManager;
class ScreenManager;
class Renderer;

class AnimationTestScreen : public IScreen
{
  private:
    enum class Direction
    {
        Left,
        Right
    };
    bool animation_finished;
    Animator *animator;
    Direction direction;
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    std::unique_ptr<Sprite> sprite;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;

  public:
    AnimationTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        Animator &animator,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~AnimationTestScreen();

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}