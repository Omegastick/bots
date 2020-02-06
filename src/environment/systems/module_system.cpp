#include <queue>

#include <entt/entt.hpp>

#include "environment/components/body.h"
#include "environment/components/modules/module.h"
#include "misc/transform.h"

namespace ai
{
void module_system(entt::registry &registry)
{
    registry.view<EcsBody, Transform>().each([&registry](auto &body, auto &transform) {
        std::queue<entt::entity> queue;
        queue.push(body.base_module);
        while (!queue.empty())
        {
            auto entity = queue.front();
            queue.pop();
            auto &module = registry.get<EcsModule>(entity);
            if (module.parent != entt::null)
            {
                registry.get<Transform>(entity) = registry.get<Transform>(module.parent);

                if (module.children > 0)
                {
                    entt::entity child = module.first;
                    for (unsigned int i = 0; i < module.children; ++i)
                    {
                        queue.push(child);
                        child = module.next;
                    }
                }
            }
            else
            {
                registry.get<Transform>(entity) = transform;
            }
        }
    });
}
}