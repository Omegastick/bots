#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "body_hit.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"

namespace SingularityTrainer
{
BodyHit::BodyHit(b2Vec2 position, glm::vec4 particle_color)
    : particle_color(particle_color),
      position(position) {}

void BodyHit::trigger(Renderer &renderer)
{
    const int particle_count = 200;
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
            0.02f,
            particle_color,
            end_color};
        particles.push_back(particle);
    }
    renderer.draw(particles);

    renderer.apply_explosive_force({position.x, position.y}, 2, 0.8f);
}
}