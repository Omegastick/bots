#include <entt/entt.hpp>

#include "square_system.h"
#include "graphics/render_data.h"

namespace ai
{
void square_system(entt::registry &registry, double delta_time)
{
    registry.view<Rectangle>().each([delta_time](auto &rectangle) {
        auto &transform = rectangle.transform;
        transform.rotate(static_cast<float>(delta_time));
        transform.move({0, -static_cast<float>(delta_time)});
        if (const auto position = transform.get_position(); position.y < -2)
        {
            transform.set_position({position.x, 12});
        }
    });
}
}