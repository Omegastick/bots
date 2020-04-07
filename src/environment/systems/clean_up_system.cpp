#include <entt/entt.hpp>

#include "clean_up_system.h"

namespace ai
{
void clean_up_system(entt::registry &registry)
{
    const auto view = registry.view<entt::tag<"should_destroy"_hs>>();
    registry.destroy(view.begin(), view.end());
}
}