#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "bullet_explosion.h"
#include "audio/audio_engine.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
BulletExplosion::BulletExplosion(b2Vec2 position, glm::vec4 particle_color)
    : particle_color(particle_color),
      position(position) {}

void BulletExplosion::trigger(Renderer &renderer, IAudioEngine *audio_engine)
{
    if (audio_engine)
    {
        audio_engine->play("hit_wall");
    }
    const int particle_count = 100;
    const float step_subdivision = 1.f / particle_count / 10.f;
    glm::vec4 end_color = particle_color;
    end_color.a = 0;
    std::vector<Particle> particles;
    for (int i = 0; i < particle_count; ++i)
    {
        Particle particle{
            glm::vec2(position.x, position.y),
            glm::diskRand(4.f),
            -i * step_subdivision,
            0.75f,
            0.03f,
            particle_color,
            end_color};
        particles.push_back(particle);
    }
    renderer.draw(particles);

    renderer.apply_explosive_force({position.x, position.y}, 2, 0.1f);
}
}