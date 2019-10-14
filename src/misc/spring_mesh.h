#pragma once

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace SingularityTrainer
{

class SpringMesh
{
  private:
    int width;
    int height;
    int no_vertices;

    float damping;
    float friction;
    float stiffness;
    float elasticity;

    std::vector<glm::vec3> accelerations;
    std::vector<glm::vec3> offsets;
    std::vector<glm::vec3> velocities;

    void apply_spring_forces(const glm::vec3 &position_1,
                             const glm::vec3 &position_2,
                             const glm::vec3 &velocity_1,
                             const glm::vec3 &velocity_2,
                             glm::vec3 &acceleration_1,
                             glm::vec3 &acceleration_2);

  public:
    SpringMesh(int width,
               int height,
               float damping = 0.06,
               float friction = 0.98,
               float stiffness = 0.28,
               float elasticity = 0.05);

    void apply_explosive_force(glm::vec2 position, float size);
    std::vector<glm::vec2> get_vertices(float scale_x, float scale_y);
    void update();

    inline const std::vector<glm::vec3> &get_offsets() const { return offsets; }
};
}