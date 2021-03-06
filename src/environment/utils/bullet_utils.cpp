#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>

#include "environment/components/bullet.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_type.h"
#include "environment/components/trail.h"
#include "misc/transform.h"

namespace ai
{
entt::entity make_bullet(entt::registry &registry)
{
    const auto entity = registry.create();
    registry.emplace<EcsBullet>(entity, 1.f);
    auto &transform = registry.emplace<Transform>(entity);
    transform.set_scale({0.15f, 0.15f});
    registry.emplace<Trail>(entity, 0.15f);

    registry.emplace<PhysicsType>(entity, PhysicsType::Bullet);
    auto &physics_body = registry.emplace<PhysicsBody>(entity);
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

    registry.emplace<EcsCircle>(entity, 0.f);
    registry.emplace<Color>(entity, cl_white, glm::vec4{0.f, 0.f, 0.f, 0.f});

    return entity;
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
}