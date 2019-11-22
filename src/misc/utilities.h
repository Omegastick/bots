#pragma once

#include <vector>
#include <torch/torch.h>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <imgui.h>

namespace SingularityTrainer
{
glm::vec2 radial_distort(glm::vec2 coordinate, glm::vec2 resolution, float strength);
b2Vec2 rotate_point_around_point(b2Vec2 point, b2Rot angle, b2Vec2 pivot);
std::string print_tensor(torch::Tensor tensor);
glm::vec2 screen_to_world_space(glm::vec2 point, glm::vec2 resolution, glm::mat4 projection);
glm::vec2 world_to_screen_space(glm::vec2 point, glm::vec2 resolution, glm::mat4 projection);

template <typename T>
std::vector<T> interleave_vectors(const std::vector<std::vector<T>> &vectors)
{
    std::vector<T> output;
    for (unsigned int i = 0; i < vectors[0].size(); ++i)
    {
        for (const auto &vector : vectors)
        {
            output.push_back(vector[i]);
        }
    }

    return output;
}
}
