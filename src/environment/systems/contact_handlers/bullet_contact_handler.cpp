#include <entt/entt.hpp>

#include "bullet_contact_handler.h"
#include "environment/components/particle_emitter.h"
#include "environment/components/physics_type.h"
#include "graphics/colors.h"
#include "misc/transform.h"

namespace ai
{
void begin_bullet_contact(entt::registry &registry,
                          entt::entity bullet_entity,
                          entt::entity other_entity,
                          PhysicsType::Type other_type)
{
    const auto &transform = registry.get<Transform>(bullet_entity);

    const auto explosion_entity = registry.create();
    registry.assign<ParticleEmitter>(explosion_entity,
                                     transform.get_position(),
                                     100,
                                     cl_white,
                                     set_alpha(cl_white, 0),
                                     0.75f,
                                     0.03f,
                                     false);

    registry.destroy(bullet_entity);
}
}