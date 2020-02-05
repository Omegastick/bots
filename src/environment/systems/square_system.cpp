#include <entt/entt.hpp>

#include "square_system.h"
#include "environment/square.h"
#include "misc/transform.h"

namespace ai
{
void square_system(entt::registry &registry, double delta_time)
{
    auto view = registry.view<Square, Transform>();

    for (auto &entity : view)
    {
        auto &transform = registry.get<Transform>(entity);
        transform.rotate(static_cast<float>(delta_time));
        transform.move({0, -static_cast<float>(delta_time)});
        if (const auto position = transform.get_position(); position.y < -2)
        {
            transform.set_position({position.x, 12});
        }
    }
}
}