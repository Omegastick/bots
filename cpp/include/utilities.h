#pragma once

#include <SFML/Graphics.hpp>
#include <Thor/Vectors.hpp>

namespace SingularityTrainer {
sf::Vector2f radial_distort(sf::Vector2f coordinate, sf::Vector2f resolution, float strength);
float rad_to_deg(float radians);
float deg_to_rad(float radians);
b2Vec2 rotate_point_around_point(b2Vec2 point, b2Rot angle, b2Vec2 pivot);
}

