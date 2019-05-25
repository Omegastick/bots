#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class VertexArray;
class VertexBuffer;
class ElementBuffer;
class Shader;
class Texture;
class Renderer;
class ScreenManager;

class TextureTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Texture> texture;
    ScreenManager *screen_manager;
    glm::mat4 projection;
    float rotation;

  public:
    TextureTestScreen(ScreenManager *screen_manager, std::vector<std::shared_ptr<IScreen>> *screens, std::vector<std::string> *screen_names);
    ~TextureTestScreen();

    virtual void update(const double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}