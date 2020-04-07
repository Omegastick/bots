#include <vector>

#include <doctest.h>
#include <entt/entt.hpp>
#include <torch/torch.h>

#include "action_system.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/systems/physics_system.h"
#include "environment/utils/body_factories.h"
#include "environment/utils/body_utils.h"

namespace ai
{
void action_system(entt::registry &registry,
                   const std::vector<torch::Tensor> &actions,
                   entt::entity *bodies,
                   std::size_t body_count)
{
    for (std::size_t i = 0; i < body_count; i++)
    {
        const auto &body_entity = bodies[i];

        unsigned int action_index = 0;
        traverse_modules(registry, body_entity, [&](auto module_entity) {
            if (!registry.has<Activatable>(module_entity))
            {
                return;
            }

            auto &activatable = registry.get<Activatable>(module_entity);
            activatable.active = actions[i].flatten()[action_index++].item().toBool();
        });
    }
}

TEST_CASE("Action system")
{
    SUBCASE("Correctly sets actions")
    {
        entt::registry registry;
        init_physics(registry);

        std::vector<entt::entity> bodies;
        for (int i = 0; i < 2; i++)
        {
            const auto entity = make_body(registry);

            const auto base_module = registry.get<EcsBody>(entity).base_module;

            const auto module_1 = make_gun_module(registry);
            link_modules(registry, base_module, 0, module_1, 0);
            const auto module_2 = make_gun_module(registry);
            link_modules(registry, module_1, 1, module_2, 0);
            const auto module_3 = make_gun_module(registry);
            link_modules(registry, base_module, 3, module_3, 0);
            const auto module_4 = make_gun_module(registry);
            link_modules(registry, module_3, 1, module_4, 0);

            bodies.push_back(entity);
        }

        std::vector<torch::Tensor> actions = {
            torch::zeros({1, 4}, torch::TensorOptions(torch::kBool)),
            torch::zeros({1, 4}, torch::TensorOptions(torch::kBool))};
        actions[0][0][1] = true;
        actions[1][0][2] = true;

        action_system(registry, actions, bodies.data(), bodies.size());

        std::vector<bool> expected_action{false, true, false, false, false, false, true, false};
        std::vector<bool> actual_actions;

        for (const auto &body : bodies)
        {
            traverse_modules(registry, body, [&](auto module_entity) {
                if (registry.has<Activatable>(module_entity))
                {
                    actual_actions.push_back(registry.get<Activatable>(module_entity).active);
                }
            });
        }

        DOCTEST_CHECK(actual_actions == expected_action);
    }
}
}