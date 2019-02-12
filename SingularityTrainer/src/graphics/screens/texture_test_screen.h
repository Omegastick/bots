#pragma once

#include <vector>
#include <memory>
#include <string>

#include "iscreen.h"
#include "graphics/renderer.h"
#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"
#include "graphics/element_buffer.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
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

  public:
    TextureTestScreen(ScreenManager *screen_manager, std::vector<std::shared_ptr<IScreen>> *screens, std::vector<std::string> *screen_names);
    ~TextureTestScreen();

    virtual void update(const float delta_time);
    virtual void draw(bool lightweight = false);
};
}