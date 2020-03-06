#include <entt/entt.hpp>

namespace ai
{
void clean_up_system(entt::registry &registry)
{
    const auto view = registry.view<entt::tag<"should_destroy"_hs>>();
    for (const auto entity : view)
    {
        registry.destroy(entity);
    }
}
}