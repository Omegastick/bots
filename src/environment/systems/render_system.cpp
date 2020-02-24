#include <doctest.h>
#include <entt/entt.hpp>
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
}

void render_system(entt::registry &registry, Renderer &renderer)
{
    renderer.set_view(glm::ortho(0.f, 19.2f, 0.f, 10.8f));

    registry.view<EcsCircle, Transform>().each([&renderer](auto &circle, auto &transform) {
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

        auto &parent_component = registry.assign<RenderShapes>(parent_entity);
        auto &child_component_1 = registry.assign<RenderShapeContainer>(child_entity_1);
        registry.assign<RenderShapeContainer>(child_entity_2);

        parent_component.children = 2;
        parent_component.first = child_entity_1;

        child_component_1.next = child_entity_2;

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
}
}