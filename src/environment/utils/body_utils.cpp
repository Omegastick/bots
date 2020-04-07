#include <functional>
#include <limits>
#include <queue>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <spdlog/spdlog.h>

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
#include "environment/components/sensor_reading.h"
#include "environment/components/score.h"
#include "environment/systems/clean_up_system.h"
#include "environment/systems/module_system.h"
#include "environment/utils/body_factories.h"
#include "environment/utils/sensor_utils.h"

namespace glm
{
std::ostream &operator<<(std::ostream &os, const vec4 &value)
{
    os << "{" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << "}";
    return os;
}
}

namespace ai
{
void apply_color_scheme(entt::registry &registry, entt::entity body_entity)
{
    const auto &color_scheme = registry.get<ColorScheme>(body_entity);

    traverse_modules(registry, body_entity, [&](auto entity) {
        registry.emplace_or_replace<Color>(entity,
                                           color_scheme.secondary,
                                           color_scheme.primary);

        if (registry.has<RenderShapes>(entity))
        {
            const auto &render_shapes = registry.get<RenderShapes>(entity);
            entt::entity container_entity = render_shapes.first;
            for (unsigned int i = 0; i < render_shapes.children; i++)
            {
                registry.emplace_or_replace<Color>(container_entity,
                                                   color_scheme.secondary,
                                                   color_scheme.primary);
                container_entity = registry.get<RenderShapeContainer>(container_entity).next;
            }
        }

        const auto &module = registry.get<EcsModule>(entity);
        entt::entity link_entity = module.first_link;
        for (unsigned int i = 0; i < module.links; i++)
        {
            registry.emplace_or_replace<Color>(link_entity,
                                               color_scheme.primary,
                                               glm::vec4{0, 0, 0, 0});
            link_entity = registry.get<EcsModuleLink>(link_entity).next;
        }
    });
}

void destroy_body(entt::registry &registry, entt::entity body_entity)
{
    // Destroy body
    registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(body_entity);

    // Destroy modules
    traverse_modules(registry, body_entity, [&](auto module_entity) {
        destroy_module(registry, module_entity);
    });

    // Destroy health bar
    const auto &health_bar = registry.get<HealthBar>(body_entity);
    registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(health_bar.background);
    registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(health_bar.foreground);
}

void destroy_module(entt::registry &registry, entt::entity module_entity)
{
    registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(module_entity);
    auto &module = registry.get<EcsModule>(module_entity);
    if (module.prev != entt::null)
    {
        registry.get<EcsModule>(module.prev).next = module.next;
    }

    // Destroy links
    if (module.first_link != entt::null)
    {
        entt::entity link_entity = module.first_link;
        for (unsigned int i = 0; i < module.links; i++)
        {
            registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(link_entity);
            const auto &link = registry.get<EcsModuleLink>(link_entity);
            if (link.linked)
            {
                if (link.child_link_index == -1)
                {
                    auto &parent = registry.get<EcsModule>(module.parent);
                    parent.children--;
                    entt::entity parent_link = parent.first_link;
                    for (int j = 0; j < module.parent_link_index; j++)
                    {
                        parent_link = registry.get<EcsModuleLink>(parent_link).next;
                    }
                    registry.get<EcsModuleLink>(parent_link).linked = false;
                    registry.get<EcsModuleLink>(parent_link).linked_module = entt::null;
                    registry.get<EcsModuleLink>(parent_link).child_link_index = -1;
                }
                else
                {
                    entt::entity child_entity = module.first;
                    auto *child = &registry.get<EcsModule>(child_entity);
                    while (child->parent_link_index != static_cast<int>(i))
                    {
                        child_entity = child->next;
                        child = &registry.get<EcsModule>(child_entity);
                    }
                    destroy_module(registry, child_entity);
                }
            }
            link_entity = link.next;
        }
    }

    // Destroy physics shapes
    auto &physics_shapes = registry.get<PhysicsShapes>(module_entity);
    if (physics_shapes.count > 0)
    {
        entt::entity shape = physics_shapes.first;
        for (unsigned int i = 0; i < physics_shapes.count; i++)
        {
            registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(shape);
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
            registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(shape);
            shape = registry.get<RenderShapeContainer>(shape).next;
        }
    }

    // Destroy sensor readings
    if (registry.has<Sensor>(module_entity))
    {
        resize_sensor(registry, module_entity, 0);
    }
}

NearestLinkResult find_nearest_link(entt::registry &registry, entt::entity module_entity)
{
    update_link_transforms(registry, module_entity);

    double closest_distance = std::numeric_limits<float>::infinity();
    entt::entity closest_link = entt::null;
    entt::entity closest_other_module = entt::null;
    entt::entity closest_other_link = entt::null;

    const auto &module = registry.get<EcsModule>(module_entity);

    const auto view = registry.view<EcsModuleLink>();
    for (const auto &other_entity : view)
    {
        const auto &other_link = registry.get<EcsModuleLink>(other_entity);
        if (other_link.parent == module_entity || other_link.linked)
        {
            continue;
        }

        const auto &other_transform = registry.get<Transform>(other_entity);
        entt::entity link = module.first_link;
        for (unsigned int i = 0; i < module.links; i++)
        {
            const auto &link_transform = registry.get<Transform>(link);
            const auto distance = glm::distance(link_transform.get_position(),
                                                other_transform.get_position());
            if (distance < closest_distance)
            {
                closest_distance = distance;
                closest_link = link;
                closest_other_module = other_link.parent;
                closest_other_link = other_entity;
            }

            link = registry.get<EcsModuleLink>(link).next;
        }
    }

    return {closest_other_module,
            closest_other_link,
            closest_link,
            static_cast<float>(closest_distance)};
}

class GetFirstQueryCallback : public b2QueryCallback
{
  private:
    b2Fixture *fixture = nullptr;

  public:
    bool ReportFixture(b2Fixture *fixture) override
    {
        this->fixture = fixture;
        return false;
    }

    b2Fixture *get() { return fixture; }
};

unsigned int get_action_count(const entt::registry &registry, entt::entity body_entity)
{
    unsigned int action_count = 0;
    traverse_modules(registry, body_entity, [&](auto module_entity) {
        if (registry.has<Activatable>(module_entity))
        {
            action_count++;
        }
    });
    return action_count;
}

unsigned int get_observation_count(const entt::registry &registry, entt::entity body_entity)
{
    unsigned int observation_count = 0;
    traverse_modules(registry, body_entity, [&](auto module_entity) {
        if (registry.has<Sensor>(module_entity))
        {
            observation_count += registry.get<Sensor>(module_entity).count;
        }
    });
    return observation_count;
}

entt::entity get_module_at_point(entt::registry &registry, glm::vec2 point)
{
    GetFirstQueryCallback query_callback;
    registry.ctx<b2World>().QueryAABB(&query_callback, {{point.x, point.y},
                                                        {point.x, point.y}});
    const b2Fixture *fixture = query_callback.get();
    if (!fixture)
    {
        return entt::null;
    }

    const auto entity = static_cast<entt::registry::entity_type>(
        reinterpret_cast<uintptr_t>(fixture->GetUserData()));
    return entity;
}

void link_modules(entt::registry &registry, entt::entity link_a_entity, entt::entity link_b_entity)
{
    auto &link_a = registry.get<EcsModuleLink>(link_a_entity);
    auto &link_b = registry.get<EcsModuleLink>(link_b_entity);
    auto &module_a = registry.get<EcsModule>(link_a.parent);
    auto &module_b = registry.get<EcsModule>(link_b.parent);

    link_a.linked = true;
    link_a.linked_module = link_b.parent;
    link_b.linked = true;
    link_b.linked_module = link_a.parent;

    // Calculate new offset
    snap_modules(registry, link_a.parent, link_a_entity, link_b.parent, link_b_entity);

    entt::entity temp_link_entity = module_a.first_link;
    unsigned int module_a_link_index = 0;
    while (temp_link_entity != link_a_entity)
    {
        temp_link_entity = registry.get<EcsModuleLink>(temp_link_entity).next;
        module_a_link_index++;
    }
    module_b.parent_link_index = module_a_link_index;

    temp_link_entity = module_b.first_link;
    unsigned int module_b_link_index = 0;
    while (temp_link_entity != link_b_entity)
    {
        temp_link_entity = registry.get<EcsModuleLink>(temp_link_entity).next;
        module_b_link_index++;
    }
    link_a.child_link_index = module_b_link_index;

    module_b.body = module_a.body;
    module_b.parent = link_a.parent;
    module_a.children++;
    if (!registry.valid(module_a.first))
    {
        module_a.first = link_b.parent;
    }
    else
    {
        entt::entity previous_entity = module_a.first;
        auto *previous_module = &registry.get<EcsModule>(previous_entity);
        while (previous_module->next != entt::null)
        {
            previous_entity = previous_module->next;
            previous_module = &registry.get<EcsModule>(previous_entity);
        }
        previous_module->next = link_b.parent;
        module_b.prev = previous_entity;
    }

    update_body_fixtures(registry, module_a.body);
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

    link_modules(registry, link_a_entity, link_b_entity);
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

    module_b.rot_offset = glm::radians(180.f) + link_a.rot_offset - link_b.rot_offset;
    module_b.pos_offset = {glm::cos(module_b.rot_offset) * -link_b.pos_offset.x +
                               glm::sin(module_b.rot_offset) * link_b.pos_offset.y,
                           glm::sin(module_b.rot_offset) * -link_b.pos_offset.x -
                               glm::cos(module_b.rot_offset) * link_b.pos_offset.y};
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

void traverse_modules(const entt::registry &registry,
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
        if (module.children > 0)
        {
            entt::entity child = module.first;
            for (unsigned int i = 0; i < module.children; ++i)
            {
                queue.push(child);
                child = registry.get<EcsModule>(child).next;
            }
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

    traverse_modules(registry, body_entity, [&](auto module_entity) {
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
            auto shape = registry.get<PhysicsShape>(shape_entity).shape;

            std::vector<b2Vec2> points(shape.m_count);

            // Apply transform to all points in the shape
            for (int i = 0; i < shape.m_count; ++i)
            {

                b2Transform b2_transform({transform.get_position().x, transform.get_position().y},
                                         b2Rot(transform.get_rotation()));
                points[i] = b2Mul(b2_transform, shape.m_vertices[i]);
            }
            shape.Set(points.data(), shape.m_count);

            // Create the fixture
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            fixture_def.density = 1;
            fixture_def.friction = 1;
            fixture_def.restitution = 0.5f;
            fixture_def.userData = reinterpret_cast<void *>(module_entity);
            physics_body.body->CreateFixture(&fixture_def);

            shape_entity = registry.get<PhysicsShape>(shape_entity).next;
        }
    });
}

TEST_CASE("apply_color_scheme()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    const auto gun_module_entity_1 = make_gun_module(registry);
    const auto gun_module_entity_2 = make_gun_module(registry);
    auto &body = registry.get<EcsBody>(body_entity);
    link_modules(registry, body.base_module, 0, gun_module_entity_1, 1);
    link_modules(registry, gun_module_entity_1, 0, gun_module_entity_2, 1);

    const glm::vec4 primary{1.f, 0.5f, 0.f, 0.3f};
    const glm::vec4 secondary{0.f, 0.1f, 0.5f, 1.f};
    registry.get<ColorScheme>(body_entity).primary = primary;
    registry.get<ColorScheme>(body_entity).secondary = secondary;

    apply_color_scheme(registry, body_entity);

    // Modules
    traverse_modules(registry, body_entity, [&](auto entity) {
        if (registry.has<Color>(entity))
        {
            const auto &color = registry.get<Color>(entity);
            DOCTEST_CHECK(color.fill_color == secondary);
            DOCTEST_CHECK(color.stroke_color == primary);
        }

        if (registry.has<RenderShapes>(entity))
        {
            const auto &render_shapes = registry.get<RenderShapes>(entity);
            entt::entity container_entity = render_shapes.first;
            for (unsigned int i = 0; i < render_shapes.children; i++)
            {
                const auto &color = registry.get<Color>(container_entity);
                DOCTEST_CHECK(color.fill_color == secondary);
                DOCTEST_CHECK(color.stroke_color == primary);
                container_entity = registry.get<RenderShapeContainer>(container_entity).next;
            }
        }
    });

    // Module links
    const auto module_links_view = registry.view<EcsModuleLink>();
    for (const auto &entity : module_links_view)
    {
        const auto &color = registry.get<Color>(entity);
        DOCTEST_CHECK(color.fill_color == primary);
    }
}

TEST_CASE("destroy_body()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    const auto module_entity_1 = make_gun_module(registry);
    const auto module_entity_2 = make_laser_sensor_module(registry);
    auto &body = registry.get<EcsBody>(body_entity);
    link_modules(registry, body.base_module, 0, module_entity_1, 1);
    link_modules(registry, module_entity_1, 0, module_entity_2, 0);

    SUBCASE("Destroys all entities used in a body")
    {
        destroy_body(registry, body_entity);
        clean_up_system(registry);

        DOCTEST_CHECK(registry.alive() == 0);
    }
}

TEST_CASE("find_nearest_link()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    const auto module_entity = make_gun_module(registry);
    module_system(registry);

    auto &module_transform = registry.get<Transform>(module_entity);
    module_transform.set_position({3.5f, 0.f});
    module_transform.set_rotation(glm::radians(270.f));

    update_link_transforms(registry, module_entity);

    const auto result = find_nearest_link(registry, module_entity);

    const auto &body = registry.get<EcsBody>(body_entity);
    auto expected_body_link = registry.get<EcsModule>(body.base_module).first_link;
    expected_body_link = registry.get<EcsModuleLink>(expected_body_link).next;
    expected_body_link = registry.get<EcsModuleLink>(expected_body_link).next;
    expected_body_link = registry.get<EcsModuleLink>(expected_body_link).next;
    DOCTEST_CHECK(result.link_a == expected_body_link);
    DOCTEST_CHECK(result.module_a == body.base_module);

    auto expected_module_link = registry.get<EcsModule>(module_entity).first_link;
    expected_module_link = registry.get<EcsModuleLink>(expected_module_link).next;
    DOCTEST_CHECK(result.link_b == expected_module_link);

    DOCTEST_CHECK(result.distance == doctest::Approx(2.5f));
}

TEST_CASE("get_module_at_point()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    const auto &body = registry.get<EcsBody>(body_entity);
    const auto physics_body = registry.get<PhysicsBody>(body_entity);
    physics_body.body->SetTransform({5.f, 3.f}, 0.f);

    const entt::entity null_entity = entt::null;
    DOCTEST_CHECK(get_module_at_point(registry, {0.f, 0.f}) == null_entity);
    DOCTEST_CHECK(get_module_at_point(registry, {5.f, 3.f}) == body.base_module);
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

    SUBCASE("Sets both links as linked")
    {
        const auto &parent_module = registry.get<EcsModule>(body.base_module);
        const auto &parent_link = registry.get<EcsModuleLink>(parent_module.first_link);
        DOCTEST_CHECK(parent_link.linked);

        const auto &module = registry.get<EcsModule>(gun_module_entity);
        auto link_entity = module.first_link;
        link_entity = registry.get<EcsModuleLink>(link_entity).next;
        auto &link = registry.get<EcsModuleLink>(link_entity);
        DOCTEST_CHECK(link.linked);

        DOCTEST_CHECK(parent_link.linked_module == gun_module_entity);
        DOCTEST_CHECK(link.linked_module == body.base_module);
    }

    SUBCASE("Sets child link index correctly")
    {
        const auto &module = registry.get<EcsModule>(body.base_module);
        const auto &parent_link = registry.get<EcsModuleLink>(module.first_link);
        DOCTEST_CHECK(parent_link.child_link_index == 1);
    }

    SUBCASE("Works multiple times")
    {
        const auto gun_module_entity_2 = make_gun_module(registry);
        link_modules(registry, body.base_module, 1, gun_module_entity_2, 1);
        const auto gun_module_entity_3 = make_gun_module(registry);
        link_modules(registry, body.base_module, 2, gun_module_entity_3, 1);
        const auto gun_module_entity_4 = make_gun_module(registry);
        link_modules(registry, body.base_module, 3, gun_module_entity_4, 1);
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

    SUBCASE("On the Y axis")
    {
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

    SUBCASE("On the X axis")
    {
        auto &base_module = registry.get<EcsModule>(base_module_entity);
        entt::entity link_a_entity = base_module.first_link;
        link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
        link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
        link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
        auto &gun_module = registry.get<EcsModule>(gun_module_entity);
        entt::entity link_b_entity = gun_module.first_link;

        snap_modules(registry, base_module_entity, link_a_entity, gun_module_entity, link_b_entity);

        const auto &gun_transform = registry.get<Transform>(gun_module_entity);

        DOCTEST_CHECK(gun_transform.get_position().x == doctest::Approx(1.f));
        DOCTEST_CHECK(gun_transform.get_position().y == doctest::Approx(0.167));
        DOCTEST_CHECK(angular_distance(gun_transform.get_rotation(), 0.f) < 0.001);
    }

    SUBCASE("With a rotated module")
    {
        auto &base_module = registry.get<EcsModule>(base_module_entity);
        entt::entity link_a_entity = base_module.first_link;
        link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
        link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
        link_a_entity = registry.get<EcsModuleLink>(link_a_entity).next;
        auto &gun_module = registry.get<EcsModule>(gun_module_entity);
        entt::entity link_b_entity = gun_module.first_link;
        link_b_entity = registry.get<EcsModuleLink>(link_b_entity).next;

        snap_modules(registry, base_module_entity, link_a_entity, gun_module_entity, link_b_entity);

        const auto &gun_transform = registry.get<Transform>(gun_module_entity);

        DOCTEST_CHECK(gun_transform.get_position().x == doctest::Approx(1.f));
        DOCTEST_CHECK(gun_transform.get_position().y == doctest::Approx(0.f));
        DOCTEST_CHECK(angular_distance(gun_transform.get_rotation(), glm::radians(270.f)) < 0.001);
    }
}
}