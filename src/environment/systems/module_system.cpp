#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "environment/components/body.h"
#include "environment/components/module_link.h"
#include "environment/components/modules/module.h"
#include "environment/utils/body_utils.h"
#include "misc/transform.h"

namespace ai
{
void update_link_transforms(entt::registry &registry, entt::entity module_entity)
{
    auto &module = registry.get<EcsModule>(module_entity);
    auto &transform = registry.get<Transform>(module_entity);

    entt::entity link_entity = module.first_link;
    for (unsigned int i = 0; i < module.links; ++i)
    {
        const auto &link = registry.get<EcsModuleLink>(link_entity);
        auto &link_transform = registry.get<Transform>(link_entity);
        link_transform = transform;
        link_transform.move(
            {glm::cos(link_transform.get_rotation()) * link.pos_offset.x -
                 glm::sin(link_transform.get_rotation()) * link.pos_offset.y,
             glm::sin(link_transform.get_rotation()) * link.pos_offset.x +
                 glm::cos(link_transform.get_rotation()) * link.pos_offset.y});
        link_transform.rotate(link.rot_offset);
        link_entity = link.next;
    }
}

void module_system(entt::registry &registry)
{
    // Perform a breadth first search to update all transforms
    const auto view = registry.view<EcsBody, Transform>();
    for (const auto body_entity : view)
    {
        auto &body_transform = registry.get<Transform>(body_entity);

        traverse_modules(registry, body_entity, [&](auto module_entity) {
            auto &module = registry.get<EcsModule>(module_entity);
            auto &transform = registry.get<Transform>(module_entity);

            // Update module transform
            if (module.parent != entt::null)
            {
                const auto &parent_transform = registry.get<Transform>(module.parent);
                transform.set_position(parent_transform.get_position());
                transform.set_rotation(parent_transform.get_rotation());
                transform.move({glm::cos(transform.get_rotation()) * module.pos_offset.x -
                                    glm::sin(transform.get_rotation()) * module.pos_offset.y,
                                glm::sin(transform.get_rotation()) * module.pos_offset.x +
                                    glm::cos(transform.get_rotation()) * module.pos_offset.y});
                transform.rotate(module.rot_offset);
            }
            else
            {
                transform.set_position(body_transform.get_position());
                transform.set_rotation(body_transform.get_rotation());
            }
        });
    }

    const auto modules_view = registry.view<EcsModule>();
    for (const auto &entity : modules_view)
    {
        update_link_transforms(registry, entity);
    }
}
}