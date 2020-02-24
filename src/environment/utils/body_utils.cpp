#include <queue>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest/doctest.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "body_utils.h"
#include "environment/components/body.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/modules/base_module.h"
#include "environment/components/modules/gun_module.h"
#include "environment/components/modules/module.h"
#include "environment/components/module_link.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_shape.h"
#include "environment/components/physics_shapes.h"
#include "environment/components/physics_world.h"

namespace ai
{
entt::entity create_base_module(entt::registry &registry)
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

    const auto link_entity_1 = create_module_link(registry, {0.f, 0.5f}, 180.f);

    const auto link_entity_2 = create_module_link(registry, {-0.5f, 0.f}, 270.f);
    registry.get<EcsModuleLink>(link_entity_1).next = link_entity_2;

    const auto link_entity_3 = create_module_link(registry, {0.f, -0.5f}, 0.f);
    registry.get<EcsModuleLink>(link_entity_2).next = link_entity_3;

    const auto link_entity_4 = create_module_link(registry, {0.5f, 0.f}, 90.f);
    registry.get<EcsModuleLink>(link_entity_3).next = link_entity_4;

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

entt::entity create_body(entt::registry &registry)
{
    const auto entity = registry.create();
    auto &body = registry.assign<EcsBody>(entity);
    registry.assign<Transform>(entity);

    auto &physics_body = registry.assign<PhysicsBody>(entity);
    b2BodyDef body_def;
    body_def.type = b2_dynamicBody;
    body_def.position = {9.6f, 5.4f};
    physics_body.body = registry.ctx<b2World>().CreateBody(&body_def);

    const auto base_module_entity = create_base_module(registry);
    body.base_module = base_module_entity;

    return entity;
}

entt::entity create_gun_module(entt::registry &registry)
{
    const auto entity = registry.create();
    auto &module = registry.assign<EcsModule>(entity);
    registry.assign<EcsGunModule>(entity);
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

    const auto link_entity_1 = create_module_link(registry, {0.f, 0.5f}, 180.f);

    const auto link_entity_2 = create_module_link(registry, {-0.5f, 0.f}, 270.f);
    registry.get<EcsModuleLink>(link_entity_1).next = link_entity_2;

    const auto link_entity_3 = create_module_link(registry, {0.f, -0.5f}, 0.f);
    registry.get<EcsModuleLink>(link_entity_2).next = link_entity_3;

    const auto link_entity_4 = create_module_link(registry, {0.5f, 0.f}, 90.f);
    registry.get<EcsModuleLink>(link_entity_3).next = link_entity_4;

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

entt::entity create_module_link(entt::registry &registry, glm::vec2 position, float rotation)
{
    const auto entity = registry.create();
    registry.assign<EcsModuleLink>(entity, position, glm::radians(rotation));
    registry.assign<EcsSemiCircle>(entity, 0.1f);
    registry.assign<Transform>(entity);
    return entity;
}

void link_modules(entt::registry &registry,
                  entt::entity module_a_entity,
                  unsigned int module_a_link_index,
                  entt::entity module_b_entity,
                  unsigned int module_b_link_index)
{
    auto &module_a = registry.get<EcsModule>(module_a_entity);
    entt::entity link_a_entity = module_a.first_link;
    for (unsigned int i = 0; i < module_a_link_index; ++i)
    {
        link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
    }
    auto &link_a = registry.get<EcsModuleLink>(link_a_entity);

    auto &module_b = registry.get<EcsModule>(module_b_entity);
    entt::entity link_b_entity = module_b.first_link;
    for (unsigned int i = 0; i < module_b_link_index; ++i)
    {
        link_b_entity = registry.get<EcsModuleLink>(link_b_entity).next;
    }
    auto &link_b = registry.get<EcsModuleLink>(link_b_entity);

    // Calculate new offset
    float inverse_rotation = link_a.rot_offset + glm::pi<float>();
    module_b.rot_offset = inverse_rotation - link_b.rot_offset;
    module_b.pos_offset = {glm::cos(module_b.rot_offset) * -link_b.pos_offset.x -
                               glm::sin(module_b.rot_offset) * -link_b.pos_offset.y,
                           glm::sin(module_b.rot_offset) * -link_b.pos_offset.x +
                               glm::cos(module_b.rot_offset) * -link_b.pos_offset.y};
    module_b.pos_offset += link_a.pos_offset;

    module_b.parent = module_a_entity;
    module_a.children++;
    if (module_a.first == entt::null)
    {
        module_a.first = module_b_entity;
    }
    else
    {
        entt::entity previous_entity = module_a.first;
        auto &previous_module = registry.get<EcsModule>(previous_entity);
        while (previous_module.next != entt::null)
        {
            previous_module = registry.get<EcsModule>(previous_entity);
            previous_entity = previous_module.next;
        }
        registry.get<EcsModule>(previous_entity).next = module_b_entity;
        module_b.prev = previous_entity;
    }
}

void update_body_fixtures(entt::registry &registry, entt::entity body_entity)
{
    auto &body = registry.get<EcsBody>(body_entity);
    auto &physics_body = registry.get<PhysicsBody>(body_entity);

    auto fixture = physics_body.body->GetFixtureList();
    while (fixture)
    {
        auto next_fixture = fixture->GetNext();
        physics_body.body->DestroyFixture(fixture);
        fixture = next_fixture;
    }

    std::queue<entt::entity> queue;
    queue.push(body.base_module);
    while (!queue.empty())
    {
        auto module_entity = queue.front();
        queue.pop();
        auto &module = registry.get<EcsModule>(module_entity);
        auto &transform = registry.get<Transform>(module_entity);

        // Update module transform
        if (module.parent != entt::null)
        {
            transform = registry.get<Transform>(module.parent);
            transform.move(module.pos_offset);
            transform.rotate(module.rot_offset);
        }
        else
        {
            transform = Transform();
        }

        // Add children to queue
        entt::entity child = module.first;
        for (unsigned int i = 0; i < module.children; ++i)
        {
            queue.push(child);
            child = module.next;
        }

        // Set up module fixtures
        auto &shapes = registry.get<PhysicsShapes>(module_entity);
        entt::entity shape_entity = shapes.first;
        for (unsigned int i = 0; i < shapes.count; ++i)
        {
            // Copy the module's shape
            // It's important we leave the original intact in case we need to do this again
            auto &shape = registry.get<PhysicsShape>(shape_entity);

            std::vector<b2Vec2> points;
            points.resize(shape.shape.m_count);

            // Apply transform to all points in the shape
            for (int i = 0; i < shape.shape.m_count; ++i)
            {
                b2Transform b2_transform({transform.get_position().x, transform.get_position().y},
                                         b2Rot(transform.get_rotation()));
                points[i] = b2Mul(b2_transform, shape.shape.m_vertices[i]);
            }
            shape.shape.Set(points.data(), shape.shape.m_count);

            // Create the fixture
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape.shape;
            fixture_def.density = 1;
            fixture_def.friction = 1;
            auto fixture = physics_body.body->CreateFixture(&fixture_def);
            fixture->SetUserData(&module);

            shape_entity = shape.next;
        }
    }
}
}