#include <entt/entt.hpp>

#include <glm/vec2.hpp>

#include "health_bar_system.h"
#include "environment/components/body.h"
#include "environment/components/health_bar.h"
#include "misc/transform.h"

namespace ai
{
void health_bar_system(entt::registry &registry)
{
    registry.view<HealthBar, EcsBody, Transform>().each([&](auto &health_bar,
                                                            auto &body,
                                                            auto &body_transform) {
        const auto new_position = body_transform.get_position() + glm::vec2{0.f, -1.f};

        auto &background_transform = registry.get<Transform>(health_bar.background);
        background_transform.set_position(new_position);

        auto &foreground_transform = registry.get<Transform>(health_bar.foreground);
        foreground_transform.set_position(new_position);
        foreground_transform.set_scale({3.f * (body.hp / body.max_hp), 0.2f});
    });
}
}