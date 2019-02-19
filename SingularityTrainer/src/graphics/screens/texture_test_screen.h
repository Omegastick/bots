#pragma oncegraphics / renderers / renderer

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/renderers/renderer.h"
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
    glm::mat4 projection;
    float rotation;

  public:
    TextureTestScreen(ScreenManager *screen_manager, std::vector<std::shared_ptr<IScreen>> *screens, std::vector<std::string> *screen_names);
    ~TextureTestScreen();

    virtual void update(const float delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}