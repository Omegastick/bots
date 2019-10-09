#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "grid_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/backend/shader.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "graphics/sprite.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
const int length = 100;
const int no_vertices = length * length;
const float size = 1000;
const float spring_length = size / length;

const float damping = 0.06;
const float friction = 0.98;
const float stiffness = 0.28;
const float elasticity = 0.05;

GridTestScreen::GridTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      accelerations(no_vertices, {0, 0}),
      velocities(no_vertices, {0, 0})
{
    this->resource_manager = &resource_manager;
    resource_manager.load_texture("bullet", "images/bullet.png");
    sprite = std::make_unique<Sprite>("bullet");
    sprite->set_scale(glm::vec2(3, 3));
    sprite->set_origin(sprite->get_center());

    positions.reserve(10000);
    const float half_size = size / 2;
    const glm::vec2 center = {960, 540};
    for (float i = -half_size; i < half_size; i += spring_length)
    {
        for (float j = -half_size; j < half_size; j += spring_length)
        {
            original_positions.push_back(glm::vec2{i, j} + center);
            positions.push_back(glm::vec2{i, j} + center);
        }
    }

    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
}

GridTestScreen::~GridTestScreen() {}

void GridTestScreen::update(double delta_time)
{
    display_test_dialog("Grid test", *screens, *screen_names, delta_time, *screen_manager);

    // Update vertex positions
    for (int i = 0; i < no_vertices; ++i)
    {
        if (i < length || i > no_vertices - length || i % length == 0 || i % length == length - 1)
        {
            accelerations[i] = {0, 0};
        }
        glm::vec2 velocity = velocities[i];
        velocity += accelerations[i];
        positions[i] += velocity + ((original_positions[i] - positions[i]) * elasticity);
        velocity *= friction;
        if (velocity.x < 0.001 && velocity.y < 0.001)
        {
            velocity = {0, 0};
        }
        velocities[i] = velocity;
    }

    // Reset accelerations to 0
    memset(accelerations.data(), 0.f, accelerations.size() * sizeof(accelerations[0]));

    // Apply springs horizontally
    for (int row = 0; row < length; ++row)
    {
        for (int column = 0; column < length - 1; ++column)
        {
            int index = row * length + column;

            glm::vec2 vector = positions[index] - positions[index + 1];

            float vector_length = glm::sqrt((vector.x * vector.x) + (vector.y * vector.y));
            if (vector_length <= spring_length)
            {
                continue;
            }

            vector = (vector / vector_length) * (vector_length - spring_length);
            glm::vec2 velocity = velocities[index + 1] - velocities[index];
            glm::vec2 force = stiffness * vector - velocity * damping;

            accelerations[index] += -force;
            accelerations[index + 1] += force;
        }
    }

    // Apply springs vertically
    for (int column = 0; column < length; ++column)
    {
        for (int row = 0; row < length - 1; ++row)
        {
            int index = row * length + column;

            glm::vec2 vector = positions[index] - positions[index + length];

            float vector_length = glm::sqrt((vector.x * vector.x) + (vector.y * vector.y));
            if (vector_length <= spring_length)
            {
                continue;
            }

            vector = (vector / vector_length) * (vector_length - spring_length);
            glm::vec2 velocity = velocities[index + length] - velocities[index];
            glm::vec2 force = stiffness * vector - velocity * damping;

            accelerations[index] += -force;
            accelerations[index + length] += force;
        }
    }

    if (ImGui::IsKeyPressed(GLFW_KEY_SPACE))
    {
        int column = glm::linearRand<int>(0, length);
        int row = glm::linearRand<int>(0, length);
        accelerations[row * length + column] = glm::linearRand(glm::vec2(-1000, -1000),
                                                               glm::vec2(1000, 1000));
    }
}

void GridTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();

    RenderData render_data;

    for (const auto &position : positions)
    {
        sprite->set_position(position);
        render_data.sprites.push_back(*sprite);
    }

    renderer.draw(render_data, projection, 0);
    renderer.end();
}
}