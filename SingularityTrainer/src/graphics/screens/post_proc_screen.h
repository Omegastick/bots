#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "graphics/backend/shader.h"
#include "graphics/sprite.h"
#include "graphics/backend/texture.h"
#include "graphics/renderers/renderer.h"
#include "graphics/post_proc_layer.h"
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

    unsigned int fbo, msfbo, rbo;
    std::shared_ptr<Texture> texture;
    std::unique_ptr<Sprite> post_proc_sprite;
    std::unique_ptr<PostProcLayer> post_proc_layer_1;
    std::unique_ptr<PostProcLayer> post_proc_layer_2;

  public:
    PostProcScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~PostProcScreen();

    virtual void update(const float delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}