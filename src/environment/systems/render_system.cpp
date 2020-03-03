#include <doctest.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render_system.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/render_shape_container.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
void clean_up_orphans(entt::registry &registry)
{
    registry.view<RenderShapeContainer>().each([&](const auto entity, const auto &container) {
        if (!registry.valid(container.parent))
        {
            registry.destroy(entity);
        }
    });
}

void update_container_transforms(entt::registry &registry)
{
    registry.view<RenderShapeContainer, Transform>().each([&](auto &container, auto &transform) {
        const auto parent_transform = registry.get<Transform>(container.parent);
        const auto parent_rot = parent_transform.get_rotation();
        transform.set_position({glm::cos(parent_rot) * -container.pos_offset.x -
                                    glm::sin(parent_rot) * -container.pos_offset.y,
                                glm::sin(parent_rot) * -container.pos_offset.x +
                                    glm::cos(parent_rot) * -container.pos_offset.y});
        transform.move(parent_transform.get_position());
        transform.set_rotation(parent_rot + container.rot_offset);
    });
}

void render_system(entt::registry &registry, Renderer &renderer)
{
    renderer.set_view(glm::ortho(0.f, 19.2f, 0.f, 10.8f));

    clean_up_orphans(registry);
    update_container_transforms(registry);

    registry.view<EcsCircle, Transform>()
        .each([&renderer](auto &circle, auto &transform) {
            renderer.draw(Circle{circle.radius,
                                 circle.fill_color,
                                 circle.stroke_color,
                                 circle.stroke_width,
                                 transform});
        });

    registry.view<EcsRectangle, Transform>().each([&renderer](auto &rectangle, auto &transform) {
        renderer.draw(Rectangle{rectangle.fill_color,
                                rectangle.stroke_color,
                                rectangle.stroke_width,
                                transform});
    });

    registry.view<EcsSemiCircle, Transform>().each([&renderer](auto &semi_circle,
                                                               auto &transform) {
        renderer.draw(SemiCircle{semi_circle.radius,
                                 semi_circle.fill_color,
                                 semi_circle.stroke_color,
                                 semi_circle.stroke_width,
                                 transform});
    });

    registry.view<EcsTrapezoid, Transform>().each([&renderer](auto &trapezoid, auto &transform) {
        renderer.draw(Trapezoid{trapezoid.top_width,
                                trapezoid.bottom_width,
                                trapezoid.fill_color,
                                trapezoid.stroke_color,
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

TEST_CASE("Render system")
{
    entt::registry registry;

    SUBCASE("clean_up_orphans()")
    {
        const auto parent_entity = registry.create();
        const auto child_entity_1 = registry.create();
        const auto child_entity_2 = registry.create();

        registry.assign<RenderShapes>(parent_entity, 2u, child_entity_1);
        registry.assign<RenderShapeContainer>(child_entity_1, parent_entity, child_entity_2);
        registry.assign<RenderShapeContainer>(child_entity_2, parent_entity);

        SUBCASE("Leaves non-orphaned entities")
        {
            clean_up_orphans(registry);

            DOCTEST_CHECK(registry.valid(child_entity_1));
            DOCTEST_CHECK(registry.valid(child_entity_2));
            DOCTEST_CHECK(registry.valid(parent_entity));
        }

        SUBCASE("Cleans up orphaned entities")
        {
            registry.destroy(parent_entity);

            clean_up_orphans(registry);

            DOCTEST_CHECK(!registry.valid(child_entity_1));
            DOCTEST_CHECK(!registry.valid(child_entity_2));
            DOCTEST_CHECK(!registry.valid(parent_entity));
        }
    }

    SUBCASE("update_container_transforms()")
    {
        const auto parent_entity = registry.create();
        const auto child_entity_1 = registry.create();
        const auto child_entity_2 = registry.create();

        registry.assign<RenderShapes>(parent_entity, 2u, child_entity_1);
        registry.assign<Transform>(parent_entity);
        registry.assign<Transform>(child_entity_1);
        registry.assign<Transform>(child_entity_2);
        registry.assign<RenderShapeContainer>(child_entity_1,
                                              parent_entity,
                                              child_entity_2,
                                              glm::vec2{1.f, 1.f},
                                              0.5f);
        registry.assign<RenderShapeContainer>(child_entity_2,
                                              parent_entity,
                                              entt::null,
                                              glm::vec2{-1.f, 0.f},
                                              -0.3f);

        SUBCASE("Correctly calculates child transforms")
        {
            auto &parent_transform = registry.get<Transform>(parent_entity);
            parent_transform.set_position({2.f, 3.f});
            parent_transform.set_rotation(glm::radians(180.f));
            update_container_transforms(registry);

            auto &child_transform_1 = registry.get<Transform>(child_entity_1);
            DOCTEST_CHECK(child_transform_1.get_position().x == doctest::Approx(3.f));
            DOCTEST_CHECK(child_transform_1.get_position().y == doctest::Approx(2.f));
            DOCTEST_CHECK(child_transform_1.get_rotation() ==
                          doctest::Approx(glm::radians(180.f) + 0.5f));

            auto &child_transform_2 = registry.get<Transform>(child_entity_2);
            DOCTEST_CHECK(child_transform_2.get_position().x == doctest::Approx(2.f));
            DOCTEST_CHECK(child_transform_2.get_position().y == doctest::Approx(4.f));
            DOCTEST_CHECK(child_transform_2.get_rotation() ==
                          doctest::Approx(glm::radians(180.f) - 0.3f));
        }
    }
}
}