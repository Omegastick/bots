#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/element_buffer.h"
#include "resource_manager.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
class TextTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    std::unique_ptr<Texture> atlas_texture;

  public:
    TextTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~TextTestScreen();

    virtual void update(const float delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}