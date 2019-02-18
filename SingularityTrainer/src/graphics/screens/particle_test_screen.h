#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "graphics/shader.h"
#include "graphics/renderers/renderer.h"
#include "graphics/renderers/particle_renderer.h"
#include "graphics/vertex_buffer.h"
#include "resource_manager.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
class ParticleTestScreen : public IScreen
{
  private:
    ParticleRenderer particle_renderer;
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;

  public:
    ParticleTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~ParticleTestScreen();

    virtual void update(const float delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}