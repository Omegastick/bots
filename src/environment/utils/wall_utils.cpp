#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <glm/vec2.hpp>

#include "wall_utils.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_type.h"
#include "environment/components/wall.h"
#include "misc/transform.h"

namespace ai
{
entt::entity make_wall(entt::registry &registry, glm::vec2 center, glm::vec2 size, float angle)
{
    const auto entity = registry.create();
    registry.emplace<EcsWall>(entity);
    registry.emplace<EcsRectangle>(entity);
    registry.emplace<Color>(entity, cl_white);
    auto &transform = registry.emplace<Transform>(entity);
    transform.set_scale(size);
    transform.set_position(center);
    transform.set_rotation(angle);

    registry.emplace<PhysicsType>(entity, PhysicsType::Wall);
    auto &physics_body = registry.emplace<PhysicsBody>(entity);
    b2BodyDef body_def;
    body_def.type = b2_staticBody;
    body_def.position = {center.x, center.y};
    body_def.angle = angle;
    body_def.userData = reinterpret_cast<void *>(entity);
    physics_body.body = registry.ctx<b2World>().CreateBody(&body_def);

    b2PolygonShape shape;
    shape.SetAsBox(size.x * 0.5f, size.y * 0.5f);
    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.density = 1.f;
    fixture_def.friction = 1.f;
    fixture_def.restitution = 0.1f;
    fixture_def.userData = reinterpret_cast<void *>(entity);
    physics_body.body->CreateFixture(&fixture_def);

    return entity;
}
}