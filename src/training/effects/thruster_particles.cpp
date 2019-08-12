#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <random>

#include <glm/glm.hpp>

#include "thruster_particles.h"
#include "graphics/render_data.h"
#include "misc/random.h"

namespace SingularityTrainer
{
ThrusterParticles::ThrusterParticles(b2Transform transform, glm::vec4 particle_color, Random &rng)
    : particle_color(particle_color),
      rng(rng),
      transform(transform) {}

RenderData ThrusterParticles::trigger()
{
    RenderData render_data;
    b2Transform edge_transform = b2Mul(transform, b2Transform(b2Vec2(0, -0.3), b2Rot(glm::pi<float>() / 2)));
    std::uniform_real_distribution<float> distribution(-0.5, 0.5);
    const int particle_count = 100;
    const float step_subdivision = 1.f / particle_count / 10.f;
    glm::vec4 end_color = particle_color;
    end_color.a = 0;
    for (int i = 0; i < particle_count; ++i)
    {
        float random_number = rng.next_float(distribution);
        b2Rot angle = b2Mul(edge_transform.q, b2Rot(random_number));
        Particle particle{
            glm::vec2(edge_transform.p.x + edge_transform.q.s * random_number, edge_transform.p.y - edge_transform.q.c * random_number),
            -glm::vec2(angle.c * 10, angle.s * 10),
            i * step_subdivision - 0.1f,
            1,
            0.02,
            particle_color,
            end_color};
        render_data.particles.push_back(particle);
    }
    return render_data;
}
}