#include <string>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <doctest.h>

#include "utilities.h"

namespace SingularityTrainer
{
glm::vec2 radial_distort(glm::vec2 coordinate, glm::vec2 resolution, float strength)
{
    glm::vec2 scaled_coordinate(coordinate.x / resolution.x, coordinate.y / resolution.y);
    glm::vec2 centered_coordinates = glm::vec2(scaled_coordinate.x - 0.5, scaled_coordinate.y - 0.5);

    float distortion = glm::dot(centered_coordinates, centered_coordinates) * strength;
    float distortion_mul = (1.0 + distortion) * distortion;

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

bool approx(const b2Vec2 &vector_1, const b2Vec2 &vector_2)
{
    auto x = vector_1.x == doctest::Approx(vector_2.x);
    auto y = vector_1.y == doctest::Approx(vector_2.y);
    return x && y;
}

TEST_CASE("Rotating around a point produces expected results")
{
    SUBCASE("Rotate {0, 1} 90 degrees right")
    {
        b2Vec2 expected{1, 0};
        auto result = rotate_point_around_point({0, 1}, b2Rot(glm::radians(-90.f)), {0, 0});
        INFO("Expected: {" << expected.x << ", " << expected.y << "} - Actual: {" << result.x << ", " << result.y << "}");
        CHECK(approx(result, expected));
    }

    SUBCASE("Rotate {2, 0} 90 degrees left")
    {
        b2Vec2 expected{0, 2};
        auto result = rotate_point_around_point({2, 0}, b2Rot(glm::radians(90.f)), {0, 0});
        INFO("Expected: {" << expected.x << ", " << expected.y << "} - Actual: {" << result.x << ", " << result.y << "}");
        CHECK(approx(result, expected));
    }

    SUBCASE("Rotate {5, 0} 180 degrees")
    {
        b2Vec2 expected{-5, 0};
        auto result = rotate_point_around_point({5, 0}, b2Rot(glm::radians(180.f)), {0, 0});
        INFO("Expected: {" << expected.x << ", " << expected.y << "} - Actual: {" << result.x << ", " << result.y << "}");
        CHECK(approx(result, expected));
    }

    SUBCASE("Rotate {5, 2} 180 degrees around {0, 2}")
    {
        b2Vec2 expected{-5, 2};
        auto result = rotate_point_around_point({5, 2}, b2Rot(glm::radians(180.f)), {0, 2});
        INFO("Expected: {" << expected.x << ", " << expected.y << "} - Actual: {" << result.x << ", " << result.y << "}");
        CHECK(approx(result, expected));
    }
}
}

doctest::String toString(const b2Vec2 &value)
{
    return ("{" + std::to_string(value.x) + ", " + std::to_string(value.y) + "}").c_str();
}
