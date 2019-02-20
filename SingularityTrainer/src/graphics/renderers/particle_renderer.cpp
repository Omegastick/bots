#include <memory>

#include <glad/glad.h>

#include "graphics/renderers/particle_renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/render_data.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
ParticleRenderer::ParticleRenderer(int max_particles, ResourceManager &resource_manager)
    : max_particles(max_particles), particle_count(0), current_particle_index(0), resource_manager(&resource_manager)
{
    particle_positions.resize(max_particles);
    particle_velocities.resize(max_particles);
    particle_start_times.resize(max_particles);
    particle_lives.resize(max_particles);
    particle_sizes.resize(max_particles);
    particle_start_colors.resize(max_particles);
    particle_end_colors.resize(max_particles);

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
    size_vertex_buffer = std::make_unique<VertexBuffer>(nullptr, max_particles * sizeof(float), GL_STREAM_DRAW);
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

    // Set attribute pointer for sizes
    size_vertex_buffer->bind();
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(5, 1);

    // Set attribute pointer for start colors
    start_color_vertex_buffer->bind();
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(6, 1);

    // Set attribute pointer for end colors
    end_color_vertex_buffer->bind();
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribDivisor(7, 1);
}

void ParticleRenderer::add_particles(std::vector<Particle> &particles, float time)
{
    for (const auto &particle : particles)
    {
        particle_positions[current_particle_index] = particle.start_position;
        particle_velocities[current_particle_index] = particle.velocity;
        particle_lives[current_particle_index] = particle.lifetime;
        particle_sizes[current_particle_index] = particle.size;
        particle_start_times[current_particle_index] = time + particle.start_time_offset;
        particle_start_colors[current_particle_index] = particle.start_color;
        particle_end_colors[current_particle_index] = particle.end_color;
        current_particle_index++;
        current_particle_index %= max_particles;
        particle_count = particle_count > current_particle_index ? particle_count : current_particle_index;
    }
}

void ParticleRenderer::draw(float time, glm::mat4 view)
{
    position_vertex_buffer->clear();
    position_vertex_buffer->add_sub_data(&particle_positions[0], 0, particle_count * sizeof(glm::vec2));
    velocity_vertex_buffer->clear();
    velocity_vertex_buffer->add_sub_data(&particle_velocities[0], 0, particle_count * sizeof(glm::vec2));
    start_time_vertex_buffer->clear();
    start_time_vertex_buffer->add_sub_data(&particle_start_times[0], 0, particle_count * sizeof(float));
    life_vertex_buffer->clear();
    life_vertex_buffer->add_sub_data(&particle_lives[0], 0, particle_count * sizeof(float));
    size_vertex_buffer->clear();
    size_vertex_buffer->add_sub_data(&particle_sizes[0], 0, particle_count * sizeof(float));
    start_color_vertex_buffer->clear();
    start_color_vertex_buffer->add_sub_data(&particle_start_colors[0], 0, particle_count * sizeof(glm::vec4));
    end_color_vertex_buffer->clear();
    end_color_vertex_buffer->add_sub_data(&particle_end_colors[0], 0, particle_count * sizeof(glm::vec4));

    auto shader = resource_manager->shader_store.get("particle");
    shader->bind();
    shader->set_uniform_mat4f("u_mvp", view);
    shader->set_uniform_1f("u_time", time);

    // renderer.draw(particle_engine, shader);
    vertex_array.bind();
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particle_count);
}
}