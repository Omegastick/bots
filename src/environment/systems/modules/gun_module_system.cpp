#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <fmt/format.h>
#include <glm/trigonometric.hpp>

#include "gun_module_system.h"
#include "environment/components/activatable.h"
#include "environment/components/audio_emitter.h"
#include "environment/components/body.h"
#include "environment/components/bullet.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/modules/gun_module.h"
#include "environment/components/modules/module.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_type.h"
#include "environment/utils/body_factories.h"
#include "environment/utils/body_utils.h"
#include "environment/utils/bullet_utils.h"
#include "misc/transform.h"

namespace ai
{
void gun_module_system(entt::registry &registry)
{
    const auto view = registry.view<EcsGunModule>();
    for (const auto entity : view)
    {
        auto &gun_module = registry.get<EcsGunModule>(entity);
        if (!registry.get<Activatable>(entity).active || gun_module.cooldown > 0)
        {
            gun_module.cooldown--;
            return;
        }
        gun_module.cooldown = gun_module.fire_rate;

        const auto bullet_entity = make_bullet(registry);

        auto &bullet_physics_body = registry.get<PhysicsBody>(bullet_entity);
        auto &transform = registry.get<Transform>(entity);
        const auto position = transform.get_position();
        const auto rotation = transform.get_rotation();
        const b2Vec2 offset_position{position.x - glm::sin(rotation),
                                     position.y + glm::cos(rotation)};
        bullet_physics_body.body->SetTransform(offset_position, rotation);
        registry.get<Transform>(bullet_entity)
            .set_position({offset_position.x, offset_position.y});

        constexpr float velocity = 50.f;
        bullet_physics_body.body->ApplyForceToCenter({-glm::sin(rotation) * velocity,
                                                      glm::cos(rotation) * velocity},
                                                     true);

        const auto &module = registry.get<EcsModule>(entity);
        auto &physics_body = registry.get<PhysicsBody>(module.body);
        physics_body.body->ApplyForce({glm::sin(rotation) * velocity,
                                       -glm::cos(rotation) * velocity},
                                      offset_position,
                                      true);

        const auto audio_entity = registry.create();
        registry.assign<AudioEmitter>(audio_entity, audio_id_map["fire"]);
    }
}

TEST_CASE("Gun module system")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    const auto gun_module_entity = make_gun_module(registry);
    const auto &body = registry.get<EcsBody>(body_entity);
    link_modules(registry, body.base_module, 0, gun_module_entity, 1);
    auto &transform = registry.get<Transform>(gun_module_entity);
    transform.set_rotation(glm::radians(-90.f));

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
        auto &activatable = registry.get<Activatable>(gun_module_entity);
        activatable.active = true;
        gun_module_system(registry);

        SUBCASE("Spawns a bullet")
        {
            DOCTEST_CHECK(registry.size<EcsBullet>() == 1);
        }

        SUBCASE("Spawned bullet travels in the correct direction")
        {
            const auto bullet_entity = registry.view<EcsBullet>().front();
            const auto &bullet_physics_body = registry.get<PhysicsBody>(bullet_entity);

            registry.ctx<b2World>().Step(0.1f, 1, 1);
            const auto velocity = bullet_physics_body.body->GetLinearVelocity();

            const auto info_string = fmt::format("{{x: {}, y: {}}}", velocity.x, velocity.y);
            INFO(info_string);
            DOCTEST_CHECK(velocity.x > 0.f);
            DOCTEST_CHECK(velocity.y == doctest::Approx(0.f));
        }

        SUBCASE("Cooldown is set to fire rate")
        {
            const auto &gun_module = registry.get<EcsGunModule>(gun_module_entity);
            DOCTEST_CHECK(gun_module.cooldown == gun_module.fire_rate);
        }

        SUBCASE("Can't fire again until cooldown is finished")
        {
            activatable.active = true;
            gun_module_system(registry);
            DOCTEST_CHECK(registry.size<EcsBullet>() == 1);
            activatable.active = true;
            gun_module_system(registry);
            DOCTEST_CHECK(registry.size<EcsBullet>() == 1);
            activatable.active = true;
            gun_module_system(registry);
            DOCTEST_CHECK(registry.size<EcsBullet>() == 1);
            activatable.active = true;
            gun_module_system(registry);
            DOCTEST_CHECK(registry.size<EcsBullet>() == 2);
        }
    }
}
}