#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class Renderer;
class VertexArray;
class VertexBuffer;
class ElementBuffer;
class Shader;
class ScreenManager;

class QuadScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    std::unique_ptr<Shader> shader;
    ScreenManager *screen_manager;
    glm::mat4 projection;

  public:
    QuadScreen(ScreenManager *screen_manager, std::vector<std::shared_ptr<IScreen>> *screens, std::vector<std::string> *screen_names);
    ~QuadScreen();

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}