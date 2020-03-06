#include <Box2D/Box2D.h>
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
    registry.assign<EcsBullet>(entity, 1.f);
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
}