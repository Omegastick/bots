#include <cstring>

#include <doctest.h>
#include <glm/glm.hpp>

#include "spring_mesh.h"

namespace SingularityTrainer
{
SpringMesh::SpringMesh(int width,
                       int height,
                       float damping,
                       float friction,
                       float stiffness,
                       float elasticity)
    : width(width),
      height(height),
      no_vertices(width * height),
      damping(damping),
      friction(friction),
      stiffness(stiffness),
      elasticity(elasticity),
      accelerations(no_vertices, {0, 0, 0}),
      offsets(no_vertices, {0, 0, 0}),
      velocities(no_vertices, {0, 0, 0}) {}

void SpringMesh::apply_explosive_force(glm::vec2 position, float size)
{
    int index = 0;
    for (int row = 0; row < height; ++row)
    {
        for (int column = 0; column < width; ++column)
        {
            glm::vec3 vertex_position = glm::vec3{column, row, 0} + offsets[index];
            float distance = glm::length(vertex_position - glm::vec3{position.x, position.y, 0});
            if (distance < size)
            {
                accelerations[index] = glm::vec3{0, 0, 100};
            }
            index++;
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
    if (vector_length <= 1)
    {
        return;
    }

    vector = (vector / vector_length) * (vector_length - 1);
    glm::vec3 velocity = velocity_2 - velocity_1;
    glm::vec3 force = stiffness * vector - velocity * damping;

    acceleration_1 += -force;
    acceleration_2 += force;
}

std::vector<glm::vec2> SpringMesh::get_vertices(float scale_x, float scale_y)
{
    std::vector<glm::vec2> vertices(no_vertices);
    int index = 0;
    for (int row = 0; row < height; ++row)
    {
        for (int column = 0; column < width; ++column)
        {
            vertices[index] = {(column + offsets[index].x) / (width - 1) * scale_x,
                               (row + offsets[index].y) / (height - 1) * scale_y};
            index++;
        }
    }

    return vertices;
}

void SpringMesh::update()
{
    // Update vertex positions
    for (int i = 0; i < no_vertices; ++i)
    {
        if (i < width ||
            i > no_vertices - width ||
            i % width == 0 ||
            i % width == width - 1)
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
    int index_1 = 0;
    for (int row = 0; row < height; ++row)
    {
        for (int column = 0; column < width - 1; ++column)
        {
            int index_2 = index_1 + 1;
            apply_spring_forces(glm::vec3{column, row, 0} + offsets[index_1],
                                glm::vec3{column + 1, row, 0} + offsets[index_2],
                                velocities[index_1],
                                velocities[index_2],
                                accelerations[index_1],
                                accelerations[index_2]);
            index_1++;
        }
    }

    // Apply springs vertically
    index_1 = 0;
    for (int column = 0; column < width; ++column)
    {
        for (int row = 0; row < height - 1; ++row)
        {
            int index_2 = index_1 + width;
            apply_spring_forces(glm::vec3{column, row, 0} + offsets[index_1],
                                glm::vec3{column, row + 1, 0} + offsets[index_2],
                                velocities[index_1],
                                velocities[index_2],
                                accelerations[index_1],
                                accelerations[index_2]);
            index_1++;
        }
    }
}

TEST_CASE("SpringMesh")
{
    SpringMesh spring_mesh(3, 4);

    SUBCASE("Number of vertices returned is equal to width * height")
    {
        auto vertices = spring_mesh.get_vertices(1, 1);

        CHECK(vertices.size() == 12);
    }

    SUBCASE("Number of offsets returned is equal to width * height")
    {
        auto &offsets = spring_mesh.get_offsets();

        CHECK(offsets.size() == 12);
    }

    SUBCASE("Small radius explosive force moves correct vertex")
    {
        spring_mesh.apply_explosive_force({1, 1}, 0.1);
        spring_mesh.update();

        auto &offsets = spring_mesh.get_offsets();

        CHECK(offsets[4].z > 1);
    }

    SUBCASE("Vertices are scaled correctly")
    {
        auto vertices = spring_mesh.get_vertices(5, 7);

        CHECK(vertices[0] == glm::vec2{0, 0});
        CHECK(vertices[11] == glm::vec2(5, 7));
    }
}
}