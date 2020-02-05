#include <entt/entt.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render_system.h"
#include "environment/components/ecs_render_data.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
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
}