#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <glm/vec2.hpp>

#include "hill_utils.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/hill.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_type.h"
#include "graphics/colors.h"
#include "misc/transform.h"

namespace ai
{
entt::entity make_hill(entt::registry &registry, glm::vec2 center, float size)
{
    const auto entity = registry.create();
    registry.emplace<EcsHill>(entity);
    registry.emplace<EcsCircle>(entity, 0.5f);
    registry.emplace<Color>(entity, glm::vec4{0, 0, 0, 0}, cl_white);
    auto &transform = registry.emplace<Transform>(entity);
    transform.set_scale({size * 2.f, size * 2.f});
    transform.set_position(center);

    registry.emplace<PhysicsType>(entity, PhysicsType::Hill);
    auto &physics_body = registry.emplace<PhysicsBody>(entity);
    b2BodyDef body_def;
    body_def.type = b2_staticBody;
    body_def.position = {center.x, center.y};
    body_def.userData = reinterpret_cast<void *>(entity);
    physics_body.body = registry.ctx<b2World>().CreateBody(&body_def);

    b2CircleShape shape;
    shape.m_radius = size;
    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.density = 1.f;
    fixture_def.friction = 1.f;
    fixture_def.restitution = 0.1f;
    fixture_def.userData = reinterpret_cast<void *>(entity);
    fixture_def.isSensor = true;
    physics_body.body->CreateFixture(&fixture_def);

    return entity;
}
}