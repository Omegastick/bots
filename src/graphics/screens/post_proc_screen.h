#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class Renderer;
class Shader;
class ScreenManager;
class PostProcLayer;
class ResourceManager;
class Sprite;
class Texture;

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

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}