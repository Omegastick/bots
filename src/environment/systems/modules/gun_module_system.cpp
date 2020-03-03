#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <glm/trigonometric.hpp>

#include "gun_module_system.h"
#include "environment/components/activatable.h"
#include "environment/components/bullet.h"
#include "environment/components/physics_body.h"
#include "environment/utils/body_utils.h"
#include "misc/transform.h"

namespace ai
{
void gun_module_system(entt::registry &registry)
{
}

TEST_CASE("Gun module system")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, -1});

    const auto entity = create_gun_module(registry);
    auto &physics_body = registry.get<PhysicsBody>(entity);
    const auto position = physics_body.body->GetTransform().p;
    physics_body.body->SetTransform(position, glm::radians(90.f));

    SUBCASE("When not active")
    {
        SUBCASE("Doesn't spawn a bullet")
        {
            gun_module_system(registry);

            DOCTEST_CHECK(registry.size<EcsBullet>() == 0);
        }
    }

    SUBCASE("When active")
    {
        auto &activatable = registry.get<Activatable>(entity);
        activatable.active = true;
        gun_module_system(registry);

        SUBCASE("Spawns a bullet")
        {
            DOCTEST_CHECK(registry.size<EcsBullet>() == 1);
        }

        SUBCASE("Deactivates module when finished")
        {
            activatable.active = true;
        }

        SUBCASE("Spawned bullet travels in the correct direction")
        {
            const auto bullet_entity = registry.view<EcsBullet>().front();
            const auto &bullet_physics_body = registry.get<PhysicsBody>(bullet_entity);
            const auto velocity = bullet_physics_body.body->GetLinearVelocity();

            DOCTEST_CHECK(velocity.x > 0.f);
            DOCTEST_CHECK(velocity.y == doctest::Approx(0.f));
        }
    }
}
}