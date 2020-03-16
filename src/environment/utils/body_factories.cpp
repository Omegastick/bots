#include <functional>
#include <queue>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest/doctest.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "body_factories.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/health_bar.h"
#include "environment/components/modules/base_module.h"
#include "environment/components/modules/gun_module.h"
#include "environment/components/modules/module.h"
#include "environment/components/modules/thruster_module.h"
#include "environment/components/module_link.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_shape.h"
#include "environment/components/physics_shapes.h"
#include "environment/components/physics_type.h"
#include "environment/components/physics_world.h"
#include "environment/components/render_shape_container.h"
#include "environment/components/score.h"
#include "environment/systems/clean_up_system.h"
#include "environment/utils/body_utils.h"

namespace ai
{
entt::entity make_base_module(entt::registry &registry)
{
    const auto entity = registry.create();
    auto &module = registry.assign<EcsModule>(entity);
    registry.assign<EcsBaseModule>(entity);
    registry.assign<Transform>(entity);

    registry.assign<EcsRectangle>(entity,
                                  glm::vec4{0.5f, 0.5f, 0.5f, 0.5f},
                                  cl_white,
                                  0.1f);
    registry.assign<EcsCircle>(entity,
                               0.2f,
                               glm::vec4{0, 0, 0, 0},
                               cl_white,
                               0.1f);

    const auto link_entity_1 = make_module_link(registry, {0.f, 0.5f}, 0.f);
    registry.get<EcsModuleLink>(link_entity_1).parent = entity;

    const auto link_entity_2 = make_module_link(registry, {-0.5f, 0.f}, 90.f);
    registry.get<EcsModuleLink>(link_entity_1).next = link_entity_2;
    registry.get<EcsModuleLink>(link_entity_2).parent = entity;

    const auto link_entity_3 = make_module_link(registry, {0.f, -0.5f}, 180.f);
    registry.get<EcsModuleLink>(link_entity_2).next = link_entity_3;
    registry.get<EcsModuleLink>(link_entity_3).parent = entity;

    const auto link_entity_4 = make_module_link(registry, {0.5f, 0.f}, 270.f);
    registry.get<EcsModuleLink>(link_entity_3).next = link_entity_4;
    registry.get<EcsModuleLink>(link_entity_4).parent = entity;

    module.links = 4;
    module.first_link = link_entity_1;

    // Create physics shapes
    auto &shapes = registry.assign<PhysicsShapes>(entity);
    shapes.count = 1;

    const auto shape_entity = registry.create();
    auto &shape = registry.assign<PhysicsShape>(shape_entity);
    shape.shape.SetAsBox(0.5f, 0.5f);
    shapes.first = shape_entity;

    return entity;
}

entt::entity make_body(entt::registry &registry)
{
    const auto entity = registry.create();
    auto &body = registry.assign<EcsBody>(entity);
    registry.assign<Transform>(entity);
    registry.assign<PhysicsType>(entity, PhysicsType::Body);
    registry.assign<Score>(entity);

    auto &physics_body = registry.assign<PhysicsBody>(entity);
    b2BodyDef body_def;
    body_def.type = b2_dynamicBody;
    body_def.position = {0.f, 0.f};
    body_def.userData = reinterpret_cast<void *>(entity);
    physics_body.body = registry.ctx<b2World>().CreateBody(&body_def);

    const auto base_module_entity = make_base_module(registry);
    body.base_module = base_module_entity;
    auto &base_module = registry.get<EcsModule>(base_module_entity);
    base_module.body = entity;

    const auto bar_background_entity = registry.create();
    const auto bar_foreground_entity = registry.create();
    registry.assign<HealthBar>(entity, bar_background_entity, bar_foreground_entity);
    auto &bar_background_transform = registry.assign<Transform>(bar_background_entity);
    bar_background_transform.set_scale({3.f, 0.2f});
    bar_background_transform.set_z(1);
    auto &bar_foreground_transform = registry.assign<Transform>(bar_foreground_entity);
    bar_foreground_transform.set_scale({3.f, 0.2f});
    bar_foreground_transform.set_z(2);
    registry.assign<EcsRectangle>(bar_background_entity, set_alpha(cl_base0, 0.5f));
    registry.assign<EcsRectangle>(bar_foreground_entity, cl_red);

    return entity;
}

entt::entity make_gun_module(entt::registry &registry)
{
    const auto entity = registry.create();
    auto &module = registry.assign<EcsModule>(entity);
    registry.assign<EcsGunModule>(entity);
    registry.assign<Transform>(entity);
    registry.assign<Activatable>(entity);

    // Render shapes
    const auto shape_1_entity = registry.create();
    const auto shape_2_entity = registry.create();
    registry.assign<RenderShapes>(entity, 2u, shape_1_entity);
    registry.assign<RenderShapeContainer>(shape_1_entity, entity, shape_2_entity);
    registry.assign<RenderShapeContainer>(shape_2_entity, entity);

    registry.assign<EcsRectangle>(shape_1_entity,
                                  glm::vec4{0.5f, 0.5f, 0.5f, 0.5f},
                                  cl_white,
                                  0.1f);
    auto &barrel_transform = registry.assign<Transform>(shape_1_entity);
    barrel_transform.set_scale({0.333f, 0.333});
    barrel_transform.set_origin({0.f, -0.333f});

    registry.assign<EcsRectangle>(shape_2_entity,
                                  glm::vec4{0.5f, 0.5f, 0.5f, 0.5f},
                                  cl_white,
                                  0.1f);
    auto &body_transform = registry.assign<Transform>(shape_2_entity);
    body_transform.set_scale({1.f, 0.666f});
    body_transform.set_origin({0.f, 0.167f});

    // Links
    const auto link_entity_1 = make_module_link(registry, {-0.5f, -0.167f}, 90.f);
    registry.get<EcsModuleLink>(link_entity_1).parent = entity;

    const auto link_entity_2 = make_module_link(registry, {0.f, -0.5f}, 180.f);
    registry.get<EcsModuleLink>(link_entity_1).next = link_entity_2;
    registry.get<EcsModuleLink>(link_entity_2).parent = entity;

    const auto link_entity_3 = make_module_link(registry, {0.5f, -0.167f}, 270.f);
    registry.get<EcsModuleLink>(link_entity_2).next = link_entity_3;
    registry.get<EcsModuleLink>(link_entity_3).parent = entity;

    module.links = 3;
    module.first_link = link_entity_1;

    // Create physics shapes
    auto &shapes = registry.assign<PhysicsShapes>(entity);
    shapes.count = 2;
    const auto body_shape_entity = registry.create();
    const auto barrel_shape_entity = registry.create();

    auto &body_shape = registry.assign<PhysicsShape>(body_shape_entity);
    body_shape.shape.SetAsBox(0.5f, 0.333f, b2Vec2(0, -0.167f), 0);
    shapes.first = body_shape_entity;
    body_shape.next = barrel_shape_entity;

    auto &barrel_shape = registry.assign<PhysicsShape>(barrel_shape_entity);
    barrel_shape.shape.SetAsBox(0.167f, 0.167f, b2Vec2(0, 0.167f), 0);

    return entity;
}

entt::entity make_module_link(entt::registry &registry, glm::vec2 position, float rotation)
{
    const auto entity = registry.create();
    registry.assign<EcsModuleLink>(entity, position, glm::radians(rotation));
    registry.assign<EcsSemiCircle>(entity, 0.1f);
    registry.assign<Transform>(entity);
    return entity;
}

entt::entity make_thruster_module(entt::registry &registry)
{
    const auto entity = registry.create();
    auto &module = registry.assign<EcsModule>(entity);
    registry.assign<EcsThrusterModule>(entity);
    registry.assign<Activatable>(entity);

    auto &transform = registry.assign<Transform>(entity);
    transform.set_scale({1.f, 0.25f});

    registry.assign<EcsTrapezoid>(entity,
                                  0.666f,
                                  1.f,
                                  glm::vec4{0.5f, 0.5f, 0.5f, 0.5f},
                                  cl_white,
                                  0.1f);

    const auto link_entity = make_module_link(registry, {0.f, 0.125f}, 0.f);
    module.links = 1;
    module.first_link = link_entity;

    // Create physics shapes
    auto &shapes = registry.assign<PhysicsShapes>(entity);
    shapes.count = 1;

    const auto shape_entity = registry.create();
    auto &shape = registry.assign<PhysicsShape>(shape_entity);
    b2Vec2 vertices[4];
    vertices[0] = b2Vec2(-0.333f, -0.125f);
    vertices[1] = b2Vec2(-0.5f, 0.125f);
    vertices[2] = b2Vec2(0.5f, 0.125f);
    vertices[3] = b2Vec2(0.333f, -0.125f);
    shape.shape.Set(vertices, 4);
    shapes.first = shape_entity;

    return entity;
}

TEST_CASE("make_gun_module()")
{
    entt::registry registry;

    SUBCASE("Physics shapes can be traversed")
    {
        const auto gun_module_entity = make_gun_module(registry);
        auto &shapes = registry.get<PhysicsShapes>(gun_module_entity);
        entt::entity shape_entity = shapes.first;
        for (unsigned int i = 0; i < shapes.count; ++i)
        {
            auto &shape = registry.get<PhysicsShape>(shape_entity);
            shape_entity = shape.next;
        }
    }
}
}