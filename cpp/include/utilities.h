#include <SFML/Graphics.hpp>
#include <Thor/Vectors.hpp>

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