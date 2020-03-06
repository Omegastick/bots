#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "particle_system.h"
#include "graphics/renderers/renderer.h"
#include "environment/components/particle_emitter.h"

namespace ai
{
void particle_system(entt::registry &registry, Renderer &renderer)
{
    registry.view<ParticleEmitter>().each([&](auto entity, auto &emitter) {
        const float step_subdivision = 1.f / emitter.particle_count / 10.f;
        std::vector<Particle> particles(emitter.particle_count);
        for (unsigned int i = 0; i < emitter.particle_count; i++)
        {
            particles[i] = {glm::vec2(emitter.position.x, emitter.position.y),
                            glm::diskRand(4.f),
                            static_cast<float>(i) * -step_subdivision,
                            emitter.lifetime,
                            emitter.size,
                            emitter.start_color,
                            emitter.end_color};
        }
        renderer.draw(particles);

        if (!emitter.loop)
        {
            registry.assign<entt::tag<"should_destroy"_hs>>(entity);
        }
    });
}
}