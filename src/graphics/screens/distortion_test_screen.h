#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "graphics/post_processing/distortion_layer.h"
#include "misc/spring_mesh.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
class Sprite;
class ResourceManager;
class ScreenManager;
class Renderer;
class VertexArray;
class VertexBuffer;
class ElementBuffer;
class Texture;

class DistortionTestScreen : public IScreen
{
  private:
    ResourceManager &resource_manager;
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    std::unique_ptr<Sprite> sprite;
    ScreenManager *screen_manager;
    glm::mat4 projection;

    std::unique_ptr<DistortionLayer> distortion_layer;

  public:
    DistortionTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}