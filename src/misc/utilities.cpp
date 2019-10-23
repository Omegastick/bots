#include <string>
#include <sstream>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <glm/glm.hpp>
#include <torch/torch.h>

#include "misc/utilities.h"

namespace SingularityTrainer
{
glm::vec2 radial_distort(glm::vec2 coordinate, glm::vec2 resolution, float strength)
{
    glm::vec2 scaled_coordinate(coordinate.x / resolution.x, coordinate.y / resolution.y);
    glm::vec2 centered_coordinates = glm::vec2(scaled_coordinate.x - 0.5, scaled_coordinate.y - 0.5);

    float distortion = glm::dot(centered_coordinates, centered_coordinates) * strength;
    float distortion_mul = (1.0f + distortion) * distortion;

    float distorted_x = scaled_coordinate.x + centered_coordinates.x * distortion_mul;
    float distorted_y = scaled_coordinate.y + centered_coordinates.y * distortion_mul;

    return {distorted_x * resolution.x, distorted_y * resolution.y};
}

b2Vec2 rotate_point_around_point(b2Vec2 point, b2Rot angle, b2Vec2 pivot)
{
    // Translate point to pivot
    point.x -= pivot.x;
    point.y -= pivot.y;

    // Rotate point
    point = b2Mul(angle, point);

    // Remove offset
    point.x += pivot.x;
    point.y += pivot.y;

    return point;
}

std::string print_tensor(torch::Tensor tensor)
{
    std::stringstream string_stream;
    string_stream << "\n";
    auto tensor_sizes = tensor.sizes();

    if (tensor.numel() == 1)
    {
        string_stream << "[" << tensor.item().toFloat() << "]"
                      << "\n";
    }
    else if (tensor_sizes.size() > 2)
    {
        string_stream << "Tensor too large\n";
    }
    else
    {
        string_stream << std::fixed << std::setprecision(2);
        for (int i = 0; i < tensor_sizes[0]; ++i)
        {
            string_stream << "[";
            for (int j = 0; j < tensor_sizes[1]; ++j)
            {
                string_stream << tensor[i][j].item().toFloat() << " ";
            }
            string_stream << "]\n";
        }
    }

    string_stream << "Size: " << tensor.sizes();
    return string_stream.str();
}

bool approx(const b2Vec2 &vector_1, const b2Vec2 &vector_2)
{
    auto x = vector_1.x == doctest::Approx(vector_2.x);
    auto y = vector_1.y == doctest::Approx(vector_2.y);
    return x && y;
}

ImVec4 glm_to_im(const glm::vec4 &in)
{
    return {in.x, in.y, in.z, in.w};
}

glm::vec2 screen_to_world_space(glm::vec2 point, glm::vec2 resolution, glm::mat4 projection)
{
    point = point / resolution;
    point -= 0.5;
    return {point.x * (1. / projection[0][0]) * 2, point.y * (1. / projection[1][1]) * 2};
}

glm::vec2 world_to_screen_space(glm::vec2 point, glm::vec2 resolution, glm::mat4 projection)
{
    auto projected_vec4 = projection * glm::vec4(point.x, point.y, 0, 0);
    point = {projected_vec4.x, projected_vec4.y};
    point *= 0.5;
    point += 0.5;
    return point * resolution;
}

TEST_CASE("Rotating around a point produces expected results")
{
    SUBCASE("Rotate {0, 1} 90 degrees right")
    {
        b2Vec2 expected{1, 0};
        auto result = rotate_point_around_point({0, 1}, b2Rot(glm::radians(-90.f)), {0, 0});
        INFO("Expected: {" << expected.x << ", " << expected.y << "} - Actual: {" << result.x << ", " << result.y << "}");
        DOCTEST_CHECK(approx(result, expected));
    }

    SUBCASE("Rotate {2, 0} 90 degrees left")
    {
        b2Vec2 expected{0, 2};
        auto result = rotate_point_around_point({2, 0}, b2Rot(glm::radians(90.f)), {0, 0});
        INFO("Expected: {" << expected.x << ", " << expected.y << "} - Actual: {" << result.x << ", " << result.y << "}");
        DOCTEST_CHECK(approx(result, expected));
    }

    SUBCASE("Rotate {5, 0} 180 degrees")
    {
        b2Vec2 expected{-5, 0};
        auto result = rotate_point_around_point({5, 0}, b2Rot(glm::radians(180.f)), {0, 0});
        INFO("Expected: {" << expected.x << ", " << expected.y << "} - Actual: {" << result.x << ", " << result.y << "}");
        DOCTEST_CHECK(approx(result, expected));
    }

    SUBCASE("Rotate {5, 2} 180 degrees around {0, 2}")
    {
        b2Vec2 expected{-5, 2};
        auto result = rotate_point_around_point({5, 2}, b2Rot(glm::radians(180.f)), {0, 2});
        INFO("Expected: {" << expected.x << ", " << expected.y << "} - Actual: {" << result.x << ", " << result.y << "}");
        DOCTEST_CHECK(approx(result, expected));
    }
}
}

doctest::String toString(const b2Vec2 &value)
{
    return ("{" + std::to_string(value.x) + ", " + std::to_string(value.y) + "}").c_str();
}
