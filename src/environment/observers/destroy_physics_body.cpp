#include <Box2D/Box2D.h>
#include <entt/entt.hpp>

#include "environment/components/physics_body.h"

namespace ai
{
void destroy_physics_body(entt::registry &registry, entt::entity entity)
{
    auto &body = registry.get<PhysicsBody>(entity);
    registry.ctx<b2World>().DestroyBody(body.body);
}
}