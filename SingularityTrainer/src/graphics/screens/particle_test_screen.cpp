#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/particle_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/shader.h"
#include "graphics/renderer.h"
#include "graphics/vertex_buffer.h"
#include "resource_manager.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
ParticleTestScreen::ParticleTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)), max_particles(10000), particle_count(1000)
{
    particle_positions.resize(max_particles);
    particle_lifetimes.resize(max_particles);
    particle_velocities.resize(max_particles);

    this->resource_manager = &resource_manager;
    resource_manager.load_shader("particle", "shaders/particle.vert", "shaders/default.frag");

    float vertex_buffer_data[] = {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1};
    quad_vertex_buffer = std::make_unique<VertexBuffer>(&vertex_buffer_data, sizeof(vertex_buffer_data));
    position_vertex_buffer = std::make_unique<VertexBuffer>(nullptr, max_particles * sizeof(glm::vec2), GL_STREAM_DRAW);

    // Set attribute pointer for quad
    vertex_array.bind();
    glEnableVertexAttribArray(0);
    quad_vertex_buffer->bind();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(0, 0);

    // Set attribute pointer for positions
    vertex_array.bind();
    glEnableVertexAttribArray(1);
    position_vertex_buffer->bind();
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(1, 1);
}

ParticleTestScreen::~ParticleTestScreen() {}

void ParticleTestScreen::update(const float delta_time)
{
    display_test_dialog("Particle test", *screens, *screen_names, delta_time, *screen_manager);

    for (int i = 0; i < particle_count; ++i)
    {
        particle_positions[i] = glm::vec2(i, i);
    }
}

void ParticleTestScreen::draw(bool lightweight)
{
    renderer.begin_frame();

    vertex_array.bind();
    auto shader = resource_manager->shader_store.get("particle");
    shader->bind();
    shader->set_uniform_mat4f("u_mvp", projection);

    position_vertex_buffer->clear();
    position_vertex_buffer->add_sub_data(&particle_positions[0], 0, particle_count * sizeof(glm::vec2));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particle_count);

    renderer.end_frame();
}
}