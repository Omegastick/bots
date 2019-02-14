#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "graphics/shader.h"
#include "graphics/sprite.h"
#include "graphics/texture.h"
#include "graphics/renderer.h"
#include "resource_manager.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
class PostProcScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    std::unique_ptr<Sprite> sprite;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;
    Renderer renderer;

    unsigned int fbo, msfbo, rbo;
    std::shared_ptr<Texture> texture;
    std::unique_ptr<Sprite> post_proc_sprite;

  public:
    PostProcScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~PostProcScreen();

    virtual void update(const float delta_time);
    virtual void draw(bool lightweight = false);
};
}