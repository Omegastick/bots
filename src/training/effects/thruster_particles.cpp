#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

#include "thruster_particles.h"
#include "audio/audio_engine.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "misc/random.h"

namespace ai
{
ThrusterParticles::ThrusterParticles(b2Transform transform, glm::vec4 particle_color)
    : particle_color(particle_color),
      transform(transform) {}

void ThrusterParticles::trigger(Renderer &renderer, IAudioEngine * /*audio_engine*/)
{
    b2Transform edge_transform = b2Mul(transform,
                                       b2Transform(b2Vec2(0, -0.3f),
                                                   b2Rot(glm::pi<float>() / 2)));
    std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
    const int particle_count = 15;
    const float step_subdivision = (1.f / 60.f) / particle_count;
    std::vector<Particle> particles;
    for (int i = 0; i < particle_count; ++i)
    {
        float random_number = glm::linearRand(-0.5f, 0.5f);
        b2Rot angle = b2Mul(edge_transform.q, b2Rot(random_number));
        Particle particle{
            glm::vec2(edge_transform.p.x + edge_transform.q.s * random_number,
                      edge_transform.p.y - edge_transform.q.c * random_number),
            -glm::vec2(angle.c * 10, angle.s * 10),
            -i * step_subdivision,
            0.8f,
            0.05f,
            particle_color,
            {0, -1, -1, -0.1f}};
        particles.push_back(particle);
    }
    renderer.draw(particles);
}
}