#include <entt/entt.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render_system.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"

namespace ai
{
void render_system(entt::registry &registry, Renderer &renderer)
{
    renderer.set_view(glm::ortho(0.f, 19.2f, 0.f, 10.8f));

    registry.view<Rectangle>().each([&renderer](auto &rectangle) {
        renderer.draw(rectangle);
    });
}
}