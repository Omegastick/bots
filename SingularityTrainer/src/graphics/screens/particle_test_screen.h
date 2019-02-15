#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "iscreen.h"
#include "graphics/shader.h"
#include "graphics/renderer.h"
#include "graphics/vertex_buffer.h"
#include "resource_manager.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
class ParticleTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> *screens;
    std::vector<std::string> *screen_names;
    ScreenManager *screen_manager;
    ResourceManager *resource_manager;
    glm::mat4 projection;
    Renderer renderer;

    VertexArray vertex_array;
    std::unique_ptr<VertexBuffer> quad_vertex_buffer;
    std::unique_ptr<VertexBuffer> position_vertex_buffer;
    std::vector<glm::vec2> particle_positions;
    std::vector<glm::vec2> particle_velocities;
    std::vector<float> particle_lifetimes;
    int max_particles;
    int particle_count;

  public:
    ParticleTestScreen(
        ScreenManager *screen_manager,
        ResourceManager &resource_manager,
        std::vector<std::shared_ptr<IScreen>> *screens,
        std::vector<std::string> *screen_names);
    ~ParticleTestScreen();

    virtual void update(const float delta_time);
    virtual void draw(bool lightweight = false);
};
}