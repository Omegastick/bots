#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
struct Particle
{
    glm::vec2 start_position;
    glm::vec2 velocity;
    float start_time;
    float lifetime;
    glm::vec4 start_color;
    glm::vec4 end_color;
};

class ParticleRenderer
{
  private:
    ResourceManager *resource_manager;
    VertexArray vertex_array;
    std::unique_ptr<VertexBuffer> quad_vertex_buffer;
    std::unique_ptr<VertexBuffer> position_vertex_buffer;
    std::unique_ptr<VertexBuffer> velocity_vertex_buffer;
    std::unique_ptr<VertexBuffer> start_time_vertex_buffer;
    std::unique_ptr<VertexBuffer> life_vertex_buffer;
    std::unique_ptr<VertexBuffer> start_color_vertex_buffer;
    std::unique_ptr<VertexBuffer> end_color_vertex_buffer;
    std::vector<glm::vec2> particle_positions;
    std::vector<glm::vec2> particle_velocities;
    std::vector<float> particle_start_times;
    std::vector<float> particle_lives;
    std::vector<glm::vec4> particle_start_colors;
    std::vector<glm::vec4> particle_end_colors;
    int max_particles;
    int particle_count;
    int current_particle_index;

  public:
    ParticleRenderer(int max_particles, ResourceManager &resource_manager);

    void add_particles(std::vector<Particle> &particles);
    void clear_particles();
    void draw(float time, glm::mat4 view);
};
}