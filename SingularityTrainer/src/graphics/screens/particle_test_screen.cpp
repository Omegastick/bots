#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/particle_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/shader.h"
#include "graphics/renderer.h"
#include "graphics/vertex_buffer.h"
#include "graphics/colors.h"
#include "resource_manager.h"
#include "screen_manager.h"

namespace SingularityTrainer
{
ParticleTestScreen::ParticleTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      max_particles(100000),
      particle_count(0),
      current_particle_index(0)
{
    particle_positions.resize(max_particles);
    particle_velocities.resize(max_particles);
    particle_start_times.resize(max_particles);
    particle_lives.resize(max_particles);
    particle_start_colors.resize(max_particles);
    particle_end_colors.resize(max_particles);

    this->resource_manager = &resource_manager;

    resource_manager.load_shader("particle", "shaders/particle.vert", "shaders/default.frag");

    // particle_engine = std::make_unique<ParticleEngine>(resource_manager.shader_store.get("particle").get());
    float vertex_buffer_data[] = {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1};
    quad_vertex_buffer = std::make_unique<VertexBuffer>(&vertex_buffer_data, sizeof(vertex_buffer_data));
    position_vertex_buffer = std::make_unique<VertexBuffer>(nullptr, max_particles * sizeof(glm::vec2), GL_STREAM_DRAW);
    velocity_vertex_buffer = std::make_unique<VertexBuffer>(nullptr, max_particles * sizeof(glm::vec2), GL_STREAM_DRAW);
    start_time_vertex_buffer = std::make_unique<VertexBuffer>(nullptr, max_particles * sizeof(float), GL_STREAM_DRAW);
    life_vertex_buffer = std::make_unique<VertexBuffer>(nullptr, max_particles * sizeof(float), GL_STREAM_DRAW);
    start_color_vertex_buffer = std::make_unique<VertexBuffer>(nullptr, max_particles * sizeof(glm::vec4), GL_STREAM_DRAW);
    end_color_vertex_buffer = std::make_unique<VertexBuffer>(nullptr, max_particles * sizeof(glm::vec4), GL_STREAM_DRAW);

    // Set attribute pointer for quad
    vertex_array.bind();
    quad_vertex_buffer->bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(0, 0);

    // Set attribute pointer for start positions
    position_vertex_buffer->bind();
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(1, 1);

    // Set attribute pointer for velocities
    velocity_vertex_buffer->bind();
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(2, 1);

    // Set attribute pointer for start times
    start_time_vertex_buffer->bind();
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(3, 1);

    // Set attribute pointer for lives
    life_vertex_buffer->bind();
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(4, 1);

    // Set attribute pointer for start colors
    start_color_vertex_buffer->bind();
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(5, 1);

    // Set attribute pointer for end colors
    end_color_vertex_buffer->bind();
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(6, 1);
}

ParticleTestScreen::~ParticleTestScreen() {}

void ParticleTestScreen::update(const float delta_time)
{
    display_test_dialog("Particle test", *screens, *screen_names, delta_time, *screen_manager);

    float time = glfwGetTime();
    for (int i = 0; i < 10; ++i)
    {
        /* 
        particle_engine->add_particle(
            glm::vec2(960, 540),
            glm::diskRand<float>(500),
            time + (float)i / 600.f,
            5,
            cl_white,
            glm::vec4(1.0, 1.0, 1.0, 0.0)
        ); */
        particle_positions[current_particle_index] = glm::vec2(960, 540);
        particle_velocities[current_particle_index] = glm::diskRand<float>(500);
        particle_start_times[current_particle_index] = time + (float)i / 600;
        particle_lives[current_particle_index] = 5;
        particle_start_colors[current_particle_index] = glm::vec4(1.0, 1.0, 1.0, 1.0);
        particle_end_colors[current_particle_index] = glm::vec4(1.0, 1.0, 1.0, 0.0);
        current_particle_index++;
        current_particle_index %= max_particles;
        particle_count = particle_count > current_particle_index ? particle_count : current_particle_index;
    }
}

void ParticleTestScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

    // particle_engine->update_buffers();
    position_vertex_buffer->clear();
    position_vertex_buffer->add_sub_data(&particle_positions[0], 0, particle_count * sizeof(glm::vec2));
    velocity_vertex_buffer->clear();
    velocity_vertex_buffer->add_sub_data(&particle_velocities[0], 0, particle_count * sizeof(glm::vec2));
    start_time_vertex_buffer->clear();
    start_time_vertex_buffer->add_sub_data(&particle_start_times[0], 0, particle_count * sizeof(float));
    life_vertex_buffer->clear();
    life_vertex_buffer->add_sub_data(&particle_lives[0], 0, particle_count * sizeof(float));
    start_color_vertex_buffer->clear();
    start_color_vertex_buffer->add_sub_data(&particle_start_colors[0], 0, particle_count * sizeof(glm::vec4));
    end_color_vertex_buffer->clear();
    end_color_vertex_buffer->add_sub_data(&particle_end_colors[0], 0, particle_count * sizeof(glm::vec4));

    auto shader = resource_manager->shader_store.get("particle");
    shader->bind();
    shader->set_uniform_mat4f("u_mvp", projection);
    shader->set_uniform_1f("u_time", glfwGetTime());

    // renderer.draw(particle_engine, shader);
    vertex_array.bind();
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particle_count);

    renderer.end();
}
}