#include <functional>
#include <queue>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest/doctest.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "body_utils.h"
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
#include "environment/utils/body_factories.h"

namespace ai
{
void destroy_body(entt::registry &registry, entt::entity body_entity)
{
    // Destroy body
    registry.assign<entt::tag<"should_destroy"_hs>>(body_entity);

    // Destroy modules
    traverse_modules(registry, body_entity, [&](entt::entity module_entity) {
        registry.assign<entt::tag<"should_destroy"_hs>>(module_entity);
        auto &module = registry.get<EcsModule>(module_entity);

        // Destroy links
        if (module.first_link != entt::null)
        {
            entt::entity link = module.first_link;
            for (unsigned int i = 0; i < module.links; i++)
            {
                registry.assign<entt::tag<"should_destroy"_hs>>(link);
                link = registry.get<EcsModuleLink>(link).next;
            }
        }

        // Destroy physics shapes
        auto &physics_shapes = registry.get<PhysicsShapes>(module_entity);
        if (physics_shapes.count > 0)
        {
            entt::entity shape = physics_shapes.first;
            for (unsigned int i = 0; i < physics_shapes.count; i++)
            {
                registry.assign<entt::tag<"should_destroy"_hs>>(shape);
                shape = registry.get<PhysicsShape>(shape).next;
            }
        }

        // Destroy render shape containers
        if (registry.has<RenderShapes>(module_entity))
        {
            auto &shape_container = registry.get<RenderShapes>(module_entity);
            entt::entity shape = shape_container.first;
            for (unsigned int i = 0; i < shape_container.children; i++)
            {
                registry.assign<entt::tag<"should_destroy"_hs>>(shape);
                shape = registry.get<RenderShapeContainer>(shape).next;
            }
        }
    });

    // Destroy health bar
    const auto &health_bar = registry.get<HealthBar>(body_entity);
    registry.assign<entt::tag<"should_destroy"_hs>>(health_bar.background);
    registry.assign<entt::tag<"should_destroy"_hs>>(health_bar.foreground);
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

    auto &module_b = registry.get<EcsModule>(module_b_entity);
    entt::entity link_b_entity = module_b.first_link;
    for (unsigned int i = 0; i < module_b_link_index; ++i)
    {
        link_b_entity = registry.get<EcsModuleLink>(link_b_entity).next;
    }

    // Calculate new offset
    snap_modules(registry, module_a_entity, link_a_entity, module_b_entity, link_b_entity);

    auto &link_a = registry.get<EcsModuleLink>(link_a_entity);
    link_a.child_link_index = module_b_link_index;

    module_b.body = module_a.body;
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

void snap_modules(entt::registry &registry,
                  entt::entity module_a_entity,
                  entt::entity link_a_entity,
                  entt::entity module_b_entity,
                  entt::entity link_b_entity)
{
    auto &link_a = registry.get<EcsModuleLink>(link_a_entity);
    auto &module_b = registry.get<EcsModule>(module_b_entity);
    auto &link_b = registry.get<EcsModuleLink>(link_b_entity);

    module_b.rot_offset = link_a.rot_offset + link_b.rot_offset + glm::pi<float>();
    module_b.pos_offset = {glm::cos(module_b.rot_offset) * -link_b.pos_offset.x -
                               glm::sin(module_b.rot_offset) * -link_b.pos_offset.y,
                           glm::sin(module_b.rot_offset) * -link_b.pos_offset.x +
                               glm::cos(module_b.rot_offset) * -link_b.pos_offset.y};
    module_b.pos_offset += link_a.pos_offset;

    const auto &transform_a = registry.get<Transform>(module_a_entity);
    auto &transform_b = registry.get<Transform>(module_b_entity);
    transform_b.set_position(transform_a.get_position());
    transform_b.set_rotation(transform_a.get_rotation());
    transform_b.move({glm::cos(transform_b.get_rotation()) * module_b.pos_offset.x -
                          glm::sin(transform_b.get_rotation()) * module_b.pos_offset.y,
                      glm::sin(transform_b.get_rotation()) * module_b.pos_offset.x +
                          glm::cos(transform_b.get_rotation()) * module_b.pos_offset.y});
    transform_b.rotate(module_b.rot_offset);
}

void traverse_modules(entt::registry &registry,
                      entt::entity body_entity,
                      std::function<void(entt::entity)> callback)
{
    const auto &body = registry.get<EcsBody>(body_entity);
    std::queue<entt::entity> queue;
    queue.push(body.base_module);
    while (!queue.empty())
    {
        const auto module_entity = queue.front();
        queue.pop();

        // Add children to queue
        const auto &module = registry.get<EcsModule>(module_entity);
        entt::entity child = module.first;
        for (unsigned int i = 0; i < module.children; ++i)
        {
            queue.push(child);
            child = registry.get<EcsModule>(child).next;
        }

        callback(module_entity);
    }
}

void update_body_fixtures(entt::registry &registry, entt::entity body_entity)
{
    auto &physics_body = registry.get<PhysicsBody>(body_entity);

    auto fixture = physics_body.body->GetFixtureList();
    while (fixture)
    {
        auto next_fixture = fixture->GetNext();
        physics_body.body->DestroyFixture(fixture);
        fixture = next_fixture;
    }

    traverse_modules(registry, body_entity, [&](entt::entity module_entity) {
        auto &module = registry.get<EcsModule>(module_entity);
        auto &transform = registry.get<Transform>(module_entity);

        // Update module transform
        if (module.parent != entt::null)
        {
            const auto &parent_transform = registry.get<Transform>(module.parent);
            transform.set_position(parent_transform.get_position());
            transform.set_rotation(parent_transform.get_rotation());
            transform.move({glm::cos(transform.get_rotation()) * module.pos_offset.x -
                                glm::sin(transform.get_rotation()) * module.pos_offset.y,
                            glm::sin(transform.get_rotation()) * module.pos_offset.x +
                                glm::cos(transform.get_rotation()) * module.pos_offset.y});
            transform.rotate(module.rot_offset);
        }
        else
        {
            transform.set_position({0.f, 0.f});
            transform.set_rotation(0.f);
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
            fixture_def.restitution = 0.5f;
            fixture_def.userData = reinterpret_cast<void *>(body_entity);
            physics_body.body->CreateFixture(&fixture_def);

            shape_entity = shape.next;
        }
    });
}

TEST_CASE("destroy_body()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    const auto gun_module_entity_1 = make_gun_module(registry);
    const auto gun_module_entity_2 = make_gun_module(registry);
    auto &body = registry.get<EcsBody>(body_entity);
    link_modules(registry, body.base_module, 0, gun_module_entity_1, 1);
    link_modules(registry, gun_module_entity_1, 0, gun_module_entity_2, 1);

    SUBCASE("Destroys all entities used in a body")
    {
        destroy_body(registry, body_entity);
        clean_up_system(registry);

        DOCTEST_CHECK(registry.alive() == 0);
    }
}

TEST_CASE("link_modules()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    const auto gun_module_entity = make_gun_module(registry);
    const auto &body = registry.get<EcsBody>(body_entity);
    link_modules(registry, body.base_module, 0, gun_module_entity, 1);

    SUBCASE("Sets body of linked module to match parent module")
    {
        const auto &module = registry.get<EcsModule>(gun_module_entity);
        DOCTEST_CHECK(module.body == body_entity);
    }

    SUBCASE("Sets child link index correctly")
    {
        const auto &module = registry.get<EcsModule>(body.base_module);
        const auto parent_link = registry.get<EcsModuleLink>(module.first_link);
        DOCTEST_CHECK(parent_link.child_link_index == 1);
    }
}

double mod(double a, double n)
{
    return a - glm::floor(a / n) * n;
}
double angular_distance(double a, double b)
{
    return glm::abs(mod(b - a + glm::pi<float>(), glm::pi<float>() * 2) - glm::pi<float>());
}

TEST_CASE("snap_modules() correctly places snapped modules")
{
    entt::registry registry;

    const auto base_module_entity = make_base_module(registry);
    const auto gun_module_entity = make_gun_module(registry);

    auto &base_module = registry.get<EcsModule>(base_module_entity);
    entt::entity link_a_entity = base_module.first_link;
    link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
    link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
    auto &gun_module = registry.get<EcsModule>(gun_module_entity);
    entt::entity link_b_entity = gun_module.first_link;
    link_b_entity = registry.get<EcsModuleLink>(link_b_entity).next;

    snap_modules(registry, base_module_entity, link_a_entity, gun_module_entity, link_b_entity);

    const auto &gun_transform = registry.get<Transform>(gun_module_entity);

    DOCTEST_CHECK(gun_transform.get_position().x == doctest::Approx(0.f));
    DOCTEST_CHECK(gun_transform.get_position().y == doctest::Approx(-1.f));
    DOCTEST_CHECK(angular_distance(gun_transform.get_rotation(), glm::radians(180.f)) < 0.001);
}
}