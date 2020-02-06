#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

#include "physics_system.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_world.h"
#include "misc/transform.h"

namespace ai
{
void physics_system(entt::registry &registry, double delta_time)
{
    auto &world = registry.ctx<b2World>();
    world.Step(static_cast<float>(delta_time), 3, 2);

    registry.view<PhysicsBody, Transform>().each([](auto &body, auto &transform) {
        const auto position = body.body->GetPosition();
        transform.set_position({position.x, position.y});

        const auto rotation = body.body->GetAngle();
        transform.set_rotation(rotation);
    });

    static int i = 0;
    if (i++ == 30)
    {
        registry.destroy(*registry.view<PhysicsBody, Transform>().begin());
    }

    spdlog::debug(world.GetBodyCount());
}
}