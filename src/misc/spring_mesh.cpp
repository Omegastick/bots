#include <cstring>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "spring_mesh.h"

namespace SingularityTrainer
{
SpringMesh::SpringMesh(int vertices_per_side,
                       float side_size,
                       glm::vec2 center,
                       float damping,
                       float friction,
                       float stiffness,
                       float elasticity)
    : vertices_per_side(vertices_per_side),
      no_vertices(vertices_per_side * vertices_per_side),
      side_size(side_size),
      spring_length(side_size / vertices_per_side),
      damping(damping),
      friction(friction),
      stiffness(stiffness),
      elasticity(elasticity),
      accelerations(no_vertices, {0, 0, 0}),
      offsets(no_vertices, {0, 0, 0}),
      velocities(no_vertices, {0, 0, 0})
{
    positions.reserve(no_vertices);
    const float half_size = side_size / 2;
    for (float i = -half_size; i < half_size; i += spring_length)
    {
        for (float j = -half_size; j < half_size; j += spring_length)
        {
            positions.push_back(glm::vec3{i, j, 0} + glm::vec3{center.x, center.y, 0});
        }
    }
}

void SpringMesh::apply_explosive_force(glm::vec2 position, float size)
{
    for (int row = 0; row < vertices_per_side; ++row)
    {
        for (int column = 0; column < vertices_per_side; ++column)
        {
            int index = row * vertices_per_side + column;
            glm::vec3 vertex_position = positions[index] + offsets[index];
            float distance = glm::length(vertex_position - glm::vec3{position.x, position.y, 0});
            if (distance < size)
            {
                accelerations[row * vertices_per_side + column] = glm::vec3{0, 0, 100};
            }
        }
    }
}

void SpringMesh::apply_spring_forces(const glm::vec3 &position_1,
                                     const glm::vec3 &position_2,
                                     const glm::vec3 &velocity_1,
                                     const glm::vec3 &velocity_2,
                                     glm::vec3 &acceleration_1,
                                     glm::vec3 &acceleration_2)
{
    glm::vec3 vector = position_1 - position_2;

    float vector_length = glm::length(vector);
    if (vector_length <= spring_length)
    {
        return;
    }

    vector = (vector / vector_length) * (vector_length - spring_length);
    glm::vec3 velocity = velocity_2 - velocity_1;
    glm::vec3 force = stiffness * vector - velocity * damping;

    acceleration_1 += -force;
    acceleration_2 += force;
}

std::vector<glm::vec2> SpringMesh::get_vertices()
{
    std::vector<glm::vec2> vertices(no_vertices);
    for (int i = 0; i < no_vertices; ++i)
    {
        vertices[i] = {positions[i].x + offsets[i].x,
                       positions[i].y + offsets[i].y};
    }
    return vertices;
}

void SpringMesh::update()
{
    // Update vertex positions
    for (int i = 0; i < no_vertices; ++i)
    {
        if (i < vertices_per_side ||
            i > no_vertices - vertices_per_side ||
            i % vertices_per_side == 0 ||
            i % vertices_per_side == vertices_per_side - 1)
        {
            accelerations[i] = {0, 0, 0};
        }
        glm::vec3 velocity = velocities[i];
        velocity += accelerations[i];
        offsets[i] += velocity + (-offsets[i] * elasticity);
        velocity *= friction;
        if (velocity.x < 0.0001 && velocity.y < 0.0001)
        {
            velocity = {0, 0, 0};
        }
        velocities[i] = velocity;
    }

    // Reset accelerations to 0
    ::memset(accelerations.data(), 0.f, accelerations.size() * sizeof(accelerations[0]));

    // Apply springs horizontally
    for (int row = 0; row < vertices_per_side; ++row)
    {
        for (int column = 0; column < vertices_per_side - 1; ++column)
        {
            int index_1 = row * vertices_per_side + column;
            int index_2 = index_1 + 1;
            apply_spring_forces(positions[index_1] + offsets[index_1],
                                positions[index_2] + offsets[index_2],
                                velocities[index_1],
                                velocities[index_2],
                                accelerations[index_1],
                                accelerations[index_2]);
        }
    }

    // Apply springs vertically
    for (int column = 0; column < vertices_per_side; ++column)
    {
        for (int row = 0; row < vertices_per_side - 1; ++row)
        {
            int index_1 = row * vertices_per_side + column;
            int index_2 = index_1 + vertices_per_side;
            apply_spring_forces(positions[index_1] + offsets[index_1],
                                positions[index_2] + offsets[index_2],
                                velocities[index_1],
                                velocities[index_2],
                                accelerations[index_1],
                                accelerations[index_2]);
        }
    }
}
}