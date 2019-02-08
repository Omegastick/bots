#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <vector>

#include "particles/iparticle_system.h"

namespace SingularityTrainer
{
struct Particle
{
    b2Vec2 start_position;
    b2Vec2 velocity;
    sf::Color start_color;
    sf::Color end_color;
    float lifetime;
};

class LinearParticleSystem : public IParticleSystem
{
  public:
    LinearParticleSystem(float min_x, float min_y, float max_x, float max_y, int max_particles);
    ~LinearParticleSystem();

    virtual void update(float delta_time);
    void add_particle(Particle &particle, float time_offset);
    virtual void draw(sf::RenderTarget &render_target, bool lightweight = false);
    virtual bool full();

  private:
    float current_time;
    int max_particles;
    float min_x;
    float min_y;
    float max_x;
    float max_y;
    std::vector<float> start_xs;
    std::vector<float> start_ys;
    std::vector<float> velocity_xs;
    std::vector<float> velocity_ys;
    std::vector<sf::Color> start_colors;
    std::vector<sf::Color> end_colors;
    std::vector<float> spawn_times;
    std::vector<float> lifetimes;

    void delete_particles(std::vector<int> indexes);
};
}