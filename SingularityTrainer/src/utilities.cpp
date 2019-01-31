#include <SFML/Graphics.hpp>
#include <Thor/Vectors.hpp>
#include <Box2D/Box2D.h>

#include "utilities.h"

namespace SingularityTrainer
{
sf::Vector2f radial_distort(sf::Vector2f coordinate, sf::Vector2f resolution, float strength)
{
    sf::Vector2f scaled_coordinate(coordinate.x / resolution.x, coordinate.y / resolution.y);
    sf::Vector2f centered_coordinates = sf::Vector2f(scaled_coordinate.x - 0.5, scaled_coordinate.y - 0.5);

    float distortion = thor::dotProduct(centered_coordinates, centered_coordinates) * strength;
    float distortion_mul = (1.0 + distortion) * distortion;

    float distorted_x = scaled_coordinate.x + centered_coordinates.x * distortion_mul;
    float distorted_y = scaled_coordinate.y + centered_coordinates.y * distortion_mul;

    return sf::Vector2f(distorted_x * resolution.x, distorted_y * resolution.y);
}

float rad_to_deg(float radians)
{
    return radians * 57.2958;
}

float deg_to_rad(float degrees)
{
    return degrees * 0.0174533;
}

b2Vec2 rotate_point_around_point(b2Vec2 point, b2Rot angle, b2Vec2 pivot)
{
    // Translate point to pivot
    point.x -= pivot.x;
    point.y -= pivot.y;

    // Rotate point
    float x_offset_rotated = (point.x * angle.c) - (point.y * angle.s);
    float y_offset_rotated = (point.x * angle.s) + (point.y * angle.c);

    // Remove offset
    point.x = x_offset_rotated + pivot.x;
    point.y = y_offset_rotated + pivot.y;

    return point;
}
}