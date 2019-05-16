#pragma once

#include <vector>

#include <Box2D/Box2D.h>
#include <glm/vec2.hpp>
#include <imgui.h>

namespace torch
{
class Tensor;
}

namespace SingularityTrainer
{
ImVec4 glm_to_im(const glm::vec4 &in);
glm::vec2 radial_distort(glm::vec2 coordinate, glm::vec2 resolution, float strength);
b2Vec2 rotate_point_around_point(b2Vec2 point, b2Rot angle, b2Vec2 pivot);
std::string print_tensor(torch::Tensor tensor);

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
