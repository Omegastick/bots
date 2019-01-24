#pragma once

#include <SFML/Graphics.hpp>
#include <Thor/Vectors.hpp>

namespace SingularityTrainer {
sf::Vector2f radial_distort(sf::Vector2f coordinate, sf::Vector2f resolution, float strength);
float rad_to_deg(float radians);
}

