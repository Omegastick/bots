#include <entt/entt.hpp>

#include "environment/components/physics_type.h"

namespace ai
{
void begin_bullet_contact(entt::registry &registry,
                          entt::entity bullet_entity,
                          entt::entity other_entity,
                          PhysicsType::Type other_type)
{
    registry.destroy(bullet_entity);
}
}