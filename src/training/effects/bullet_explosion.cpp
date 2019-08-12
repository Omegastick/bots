#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "bullet_explosion.h"
#include "graphics/render_data.h"

namespace SingularityTrainer
{
BulletExplosion::BulletExplosion(b2Vec2 position, glm::vec4 particle_color)
    : particle_color(particle_color),
      position(position) {}

RenderData BulletExplosion::trigger()
{
    RenderData render_data;
    const int particle_count = 100;
    const float step_subdivision = 1.f / particle_count / 10.f;
    glm::vec4 end_color = particle_color;
    end_color.a = 0;
    for (int i = 0; i < particle_count; ++i)
    {
        Particle particle{
            glm::vec2(position.x, position.y),
            glm::diskRand(4.f),
            -i * step_subdivision,
            0.75,
            0.02,
            particle_color,
            end_color};
        render_data.particles.push_back(particle);
    }
    return render_data;
}
}