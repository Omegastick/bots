#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <fmt/format.h>
#include <glm/trigonometric.hpp>

#include "gun_module_system.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/components/bullet.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/modules/gun_module.h"
#include "environment/components/modules/module.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_type.h"
#include "environment/components/trail.h"
#include "environment/utils/body_utils.h"
#include "misc/transform.h"

namespace ai
{
entt::entity make_bullet(entt::registry &registry)
{
    const auto entity = registry.create();
    registry.assign<EcsBullet>(entity);
    registry.assign<Transform>(entity);
    registry.assign<Trail>(entity, 0.1f);
    registry.assign<PhysicsType>(entity, PhysicsType::Bullet);

    auto &physics_body = registry.assign<PhysicsBody>(entity);
    b2BodyDef body_def;
    body_def.type = b2_dynamicBody;
    body_def.position = {0.f, 0.f};
    body_def.userData = reinterpret_cast<void *>(entity);
    physics_body.body = registry.ctx<b2World>().CreateBody(&body_def);

    b2CircleShape shape;
    shape.m_radius = 0.1f;
    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.density = 1.f;
    fixture_def.friction = 1.f;
    fixture_def.restitution = 0.9f;
    fixture_def.isSensor = false;
    fixture_def.userData = reinterpret_cast<void *>(entity);
    physics_body.body->CreateFixture(&fixture_def);
    physics_body.body->SetBullet(true);

    registry.assign<EcsCircle>(entity, 0.1f);

    return entity;
}

void gun_module_system(entt::registry &registry)
{
    const auto view = registry.view<EcsGunModule>();
    for (const auto entity : view)
    {
        if (!registry.get<Activatable>(entity).active)
        {
            return;
        }
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

        registry.get<Activatable>(entity).active = false;
    }
}

TEST_CASE("make_bullet()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto entity = make_bullet(registry);

    SUBCASE("Creates a bullet at {0, 0}")
    {
        auto &transform = registry.get<Transform>(entity);
        DOCTEST_CHECK(transform.get_position() == glm::vec2{0.f, 0.f});

        auto &physics_body = registry.get<PhysicsBody>(entity);
        const auto b2_transform = physics_body.body->GetTransform();
        DOCTEST_CHECK(b2_transform.p.x == doctest::Approx(0.f));
        DOCTEST_CHECK(b2_transform.p.y == doctest::Approx(0.f));
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

        SUBCASE("Deactivates module when finished")
        {
            DOCTEST_CHECK(activatable.active == false);
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
    }
}
}