#include <queue>

#include <entt/entt.hpp>

#include "environment/components/body.h"
#include "environment/components/module_link.h"
#include "environment/components/modules/module.h"
#include "misc/transform.h"

namespace ai
{
void module_system(entt::registry &registry)
{
    // Perform a breadth first search to update all transforms
    registry.view<EcsBody, Transform>().each([&registry](auto &body, auto &body_transform) {
        std::queue<entt::entity> queue;
        queue.push(body.base_module);
        while (!queue.empty())
        {
            auto entity = queue.front();
            queue.pop();
            auto &module = registry.get<EcsModule>(entity);
            auto &transform = registry.get<Transform>(entity);

            // Update module transform
            if (module.parent != entt::null)
            {
                transform = registry.get<Transform>(module.parent);

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
                transform = body_transform;
            }

            // Update link transforms
            if (module.links > 0)
            {
                entt::entity link_entity = module.first_link;
                for (unsigned int i = 0; i < module.links; ++i)
                {
                    const auto &link = registry.get<EcsModuleLink>(link_entity);
                    auto &link_transform = registry.get<Transform>(link_entity);
                    link_transform = transform;
                    link_transform.move(link.pos_offset);
                    link_transform.rotate(link.rot_offset);
                    link_entity = link.next;
                }
            }
        }
    });
}
}