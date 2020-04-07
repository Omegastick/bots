#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>

#include "render_system.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/render_shape_container.h"
#include "environment/systems/clean_up_system.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
void clean_up_orphans(entt::registry &registry)
{
    registry.view<RenderShapeContainer>().each([&](const auto entity, const auto &container) {
        if (!registry.valid(container.parent) ||
            registry.has<entt::tag<"should_destroy"_hs>>(container.parent))
        {
            registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(entity);
        }
    });
}

void update_container_transforms(entt::registry &registry)
{
    registry.view<RenderShapeContainer, Transform>().each([&](auto &container, auto &transform) {
        const auto parent_transform = registry.get<Transform>(container.parent);
        const auto parent_rot = parent_transform.get_rotation();
        transform.set_position({glm::cos(parent_rot) * container.pos_offset.x -
                                    glm::sin(parent_rot) * container.pos_offset.y,
                                glm::sin(parent_rot) * container.pos_offset.x +
                                    glm::cos(parent_rot) * container.pos_offset.y});
        transform.move(parent_transform.get_position());
        transform.set_rotation(parent_rot + container.rot_offset);
    });
}

void render_system(entt::registry &registry, Renderer &renderer)
{
    clean_up_orphans(registry);
    update_container_transforms(registry);
    clean_up_system(registry);

    registry.view<EcsCircle, Transform, Color>().each([&renderer](auto &circle,
                                                                  auto &transform,
                                                                  auto &color) {
        renderer.draw(Circle{transform.get_scale().x * 0.5f,
                             color.fill_color,
                             color.stroke_color,
                             circle.stroke_width,
                             transform});
    });

    registry.view<EcsRectangle, Transform, Color>().each([&renderer](auto &rectangle,
                                                                     auto &transform,
                                                                     auto &color) {
        renderer.draw(Rectangle{color.fill_color,
                                color.stroke_color,
                                rectangle.stroke_width,
                                transform});
    });

    registry.view<EcsSemiCircle, Transform, Color>().each([&renderer](auto &semi_circle,
                                                                      auto &transform,
                                                                      auto &color) {
        renderer.draw(SemiCircle{transform.get_scale().x * 0.5f,
                                 color.fill_color,
                                 color.stroke_color,
                                 semi_circle.stroke_width,
                                 transform});
    });

    registry.view<EcsTrapezoid, Transform, Color>().each([&renderer](auto &trapezoid,
                                                                     auto &transform,
                                                                     auto &color) {
        renderer.draw(Trapezoid{trapezoid.top_width,
                                trapezoid.bottom_width,
                                color.fill_color,
                                color.stroke_color,
                                trapezoid.stroke_width,
                                transform});
    });

    registry.view<Line>().each([&renderer](auto &line) {
        renderer.draw(line);
    });

    registry.view<Sprite>().each([&renderer](auto &sprite) {
        renderer.draw(sprite);
    });

    registry.view<Text>().each([&renderer](auto &text) {
        renderer.draw(text);
    });
}

void debug_render_system(entt::registry &registry, Renderer &renderer)
{
    auto &world = registry.ctx<b2World>();
    b2Body *body = world.GetBodyList();
    while (body)
    {
        const auto body_transform = body->GetTransform();
        const auto rot = body_transform.q.GetAngle();
        b2Fixture *fixture = body->GetFixtureList();
        while (fixture)
        {
            if (fixture->GetType() != b2Shape::Type::e_polygon)
            {
                fixture = fixture->GetNext();
                continue;
            }

            Circle circle{0.1f};
            auto *shape = static_cast<b2PolygonShape *>(fixture->GetShape());
            for (int i = 0; i < shape->m_count; i++)
            {
                const auto vertex = shape->m_vertices[i];
                circle.transform.set_position({body_transform.p.x, body_transform.p.y});
                circle.transform.move({glm::cos(rot) * vertex.x - glm::sin(rot) * vertex.y,
                                       glm::sin(rot) * vertex.x + glm::cos(rot) * vertex.y});
                renderer.draw(circle);
            }

            fixture = fixture->GetNext();
        }
        body = body->GetNext();
    }
}

TEST_CASE("Render system")
{
    entt::registry registry;

    SUBCASE("clean_up_orphans()")
    {
        const auto parent_entity = registry.create();
        const auto child_entity_1 = registry.create();
        const auto child_entity_2 = registry.create();

        registry.emplace<RenderShapes>(parent_entity, 2u, child_entity_1);
        registry.emplace<RenderShapeContainer>(child_entity_1, parent_entity, child_entity_2);
        registry.emplace<RenderShapeContainer>(child_entity_2, parent_entity);

        SUBCASE("Leaves non-orphaned entities")
        {
            clean_up_orphans(registry);

            DOCTEST_CHECK(registry.valid(child_entity_1));
            DOCTEST_CHECK(registry.valid(child_entity_2));
            DOCTEST_CHECK(registry.valid(parent_entity));
        }

        SUBCASE("Cleans up orphaned entities")
        {
            registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(parent_entity);

            clean_up_orphans(registry);

            DOCTEST_CHECK(registry.has<entt::tag<"should_destroy"_hs>>(child_entity_1));
            DOCTEST_CHECK(registry.has<entt::tag<"should_destroy"_hs>>(child_entity_2));
            DOCTEST_CHECK(registry.has<entt::tag<"should_destroy"_hs>>(parent_entity));
        }
    }

    SUBCASE("update_container_transforms()")
    {
        const auto parent_entity = registry.create();
        const auto child_entity_1 = registry.create();
        const auto child_entity_2 = registry.create();

        registry.emplace<RenderShapes>(parent_entity, 2u, child_entity_1);
        registry.emplace<Transform>(parent_entity);
        registry.emplace<Transform>(child_entity_1);
        registry.emplace<Transform>(child_entity_2);
        registry.emplace<RenderShapeContainer>(child_entity_1,
                                               parent_entity,
                                               child_entity_2,
                                               glm::vec2{1.f, 1.f},
                                               0.5f);
        registry.emplace<RenderShapeContainer>(child_entity_2,
                                               parent_entity,
                                               entt::null,
                                               glm::vec2{-1.f, 0.f},
                                               -0.3f);

        SUBCASE("Correctly calculates child transforms")
        {
            auto &parent_transform = registry.get<Transform>(parent_entity);
            parent_transform.set_position({2.f, 3.f});
            parent_transform.set_rotation(glm::radians(270.f));
            update_container_transforms(registry);

            auto &child_transform_1 = registry.get<Transform>(child_entity_1);
            DOCTEST_CHECK(child_transform_1.get_position().x == doctest::Approx(3.f));
            DOCTEST_CHECK(child_transform_1.get_position().y == doctest::Approx(2.f));
            DOCTEST_CHECK(child_transform_1.get_rotation() ==
                          doctest::Approx(glm::radians(270.f) + 0.5f));

            auto &child_transform_2 = registry.get<Transform>(child_entity_2);
            DOCTEST_CHECK(child_transform_2.get_position().x == doctest::Approx(2.f));
            DOCTEST_CHECK(child_transform_2.get_position().y == doctest::Approx(4.f));
            DOCTEST_CHECK(child_transform_2.get_rotation() ==
                          doctest::Approx(glm::radians(270.f) - 0.3f));
        }
    }
}
}