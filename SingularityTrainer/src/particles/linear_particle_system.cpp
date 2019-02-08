#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <vector>

#include "particles/iparticle_system.h"
#include "particles/linear_particle_system.h"

inline float lerp(float start, float end, float percent)
{
    return start + percent * (end - start);
}

namespace SingularityTrainer
{
LinearParticleSystem::LinearParticleSystem(float min_x, float min_y, float max_x, float max_y, int max_particles)
    : min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y), max_particles(max_particles), current_time(0) {}
LinearParticleSystem::~LinearParticleSystem() {}

void LinearParticleSystem::update(float delta_time)
{
    current_time += delta_time;
}

void LinearParticleSystem::add_particle(Particle &particle, float time_offset)
{
    if (start_xs.size() > max_particles)
    {
        return;
    }
    start_xs.push_back(particle.start_position.x);
    start_ys.push_back(particle.start_position.y);
    velocity_xs.push_back(particle.velocity.x);
    velocity_ys.push_back(particle.velocity.y);
    start_colors.push_back(particle.start_color);
    end_colors.push_back(particle.end_color);
    lifetimes.push_back(particle.lifetime);
    spawn_times.push_back(current_time + time_offset);
}

void LinearParticleSystem::draw(sf::RenderTarget &render_target, bool lightweight)
{
    int count = start_xs.size();

    // Delete expired particles
    std::vector<int> delete_indexes;
    for (int i = 0; i < count; ++i)
    {
        if (lifetimes[i] < current_time - spawn_times[i])
        {
            delete_indexes.push_back(i);
        }
    }
    delete_particles(delete_indexes);

    // Update count
    count = start_xs.size();

    float xs[count];
    float ys[count];
    float lives[count];
    sf::Color colors[count];

    // Calculate lives
    for (int i = 0; i < count; ++i)
    {
        lives[i] = current_time - spawn_times[i];
    }

    // Calculate particle X positions
    for (int i = 0; i < count; ++i)
    {
        xs[i] = start_xs[i] + (lives[i] * velocity_xs[i]);
        xs[i] = xs[i] > max_x ? max_x : xs[i] < min_x ? min_x : xs[i];
    }

    // Calculate particle Y positions
    for (int i = 0; i < count; ++i)
    {
        ys[i] = start_ys[i] + (lives[i] * velocity_ys[i]);
        ys[i] = ys[i] > max_y ? max_y : ys[i] < min_y ? min_y : ys[i];
    }

    // Calculate colours
    for (int i = 0; i < count; ++i)
    {
        sf::Color color;
        if (lives[i] > 0)
        {
            float life_usage = lives[i] / lifetimes[i];
            color.r = lerp(start_colors[i].r, end_colors[i].r, life_usage);
            color.g = lerp(start_colors[i].g, end_colors[i].g, life_usage);
            color.b = lerp(start_colors[i].b, end_colors[i].b, life_usage);
            color.a = lerp(start_colors[i].a, end_colors[i].a, life_usage);
        }
        else
        {
            color = sf::Color::Transparent;
        }
        colors[i] = color;
    }

    // Create vertices
    sf::VertexArray vertices(sf::Quads, count * 4);
    for (int i = 0; i < count; ++i)
    {
        vertices[i * 4].position = sf::Vector2f(xs[i] - 0.1, ys[i] - 0.1);
        vertices[i * 4].color = colors[i];
        vertices[i * 4 + 1].position = sf::Vector2f(xs[i] + 0.1, ys[i] - 0.1);
        vertices[i * 4 + 1].color = colors[i];
        vertices[i * 4 + 2].position = sf::Vector2f(xs[i] + 0.1, ys[i] + 0.1);
        vertices[i * 4 + 2].color = colors[i];
        vertices[i * 4 + 3].position = sf::Vector2f(xs[i] - 0.1, ys[i] + 0.1);
        vertices[i * 4 + 3].color = colors[i];
    }

    render_target.draw(vertices);
}

void LinearParticleSystem::delete_particles(std::vector<int> indexes)
{
    for (const auto i : indexes)
    {
        start_xs[i] = start_xs.back();
        start_xs.pop_back();
    }
    for (const auto i : indexes)
    {
        start_ys[i] = start_ys.back();
        start_ys.pop_back();
    }
    for (const auto i : indexes)
    {
        velocity_xs[i] = velocity_xs.back();
        velocity_xs.pop_back();
    }
    for (const auto i : indexes)
    {
        velocity_ys[i] = velocity_ys.back();
        velocity_ys.pop_back();
    }
    for (const auto i : indexes)
    {
        start_colors[i] = start_colors.back();
        start_colors.pop_back();
    }
    for (const auto i : indexes)
    {
        end_colors[i] = end_colors.back();
        end_colors.pop_back();
    }
    for (const auto i : indexes)
    {
        lifetimes[i] = lifetimes.back();
        lifetimes.pop_back();
    }
    for (const auto i : indexes)
    {
        spawn_times[i] = spawn_times.back();
        spawn_times.pop_back();
    }
}

bool LinearParticleSystem::full()
{
    return start_xs.size() >= max_particles;
}
}
