#include <entt/entt.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render_system.h"
#include "environment/square.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "misc/transform.h"

namespace ai
{
void render_system(entt::registry &registry, Renderer &renderer)
{
    renderer.set_view(glm::ortho(0.f, 19.2f, 0.f, 10.8f));

    registry.view<Square, Transform>().each([&renderer](auto &square, auto &transform) {
        renderer.draw(Rectangle{set_alpha(square.color, 0.5f),
                                square.color,
                                0.1f,
                                transform});
    });
}
}