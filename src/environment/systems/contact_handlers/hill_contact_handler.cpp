#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>

#include "hill_contact_handler.h"
#include "environment/components/hill.h"
#include "environment/components/physics_type.h"
#include "environment/components/modules/module.h"
#include "environment/utils/hill_utils.h"

namespace ai
{
void begin_hill_contact(entt::registry &registry,
                        entt::entity hill_entity,
                        entt::entity other_entity,
                        PhysicsType::Type other_type)
{
    if (other_type != PhysicsType::Module)
    {
        return;
    }

    const auto body = registry.get<EcsModule>(other_entity).body;

    auto &hill = registry.get<EcsHill>(hill_entity);
    auto occupant = std::find_if(hill.occupants.begin(),
                                 hill.occupants.end(),
                                 [&](const auto &occupant) { return occupant.first == body; });
    if (occupant == hill.occupants.end())
    {
        hill.occupants[hill_occupant_count(hill)] = {body, 1};
    }
    else
    {
        occupant->second++;
    }
}

void end_hill_contact(entt::registry &registry,
                      entt::entity hill_entity,
                      entt::entity other_entity,
                      PhysicsType::Type other_type)
{
    if (other_type != PhysicsType::Module)
    {
        return;
    }

    const auto body = registry.get<EcsModule>(other_entity).body;

    auto &hill = registry.get<EcsHill>(hill_entity);
    for (auto &occupant : hill.occupants)
    {
        if (occupant.first == body)
        {
            if (occupant.second > 0)
            {
                occupant.second--;
            }
            else
            {
                occupant = {entt::null, 0};
            }
        }
    }
}

TEST_CASE("Hill contact handler")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto hill_entity = make_hill(registry, {0.f, 0.f}, 1.f);

    const auto body_1 = registry.create();
    const auto module_1 = registry.create();
    registry.emplace<EcsModule>(module_1, body_1);

    const auto body_2 = registry.create();
    const auto module_2 = registry.create();
    registry.emplace<EcsModule>(module_2, body_2);

    SUBCASE("Counts up to two bodies on the hill")
    {
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 0);

        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 1);

        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 2);
    }

    SUBCASE("Counts down as bodies leave the hill")
    {
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 2);

        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 1);

        end_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 0);
    }

    SUBCASE("The same body entering the hill twice does nothing")
    {
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 1);
    }

    SUBCASE("The same body leaving the hill twice does nothing")
    {
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 2);

        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 1);

        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 1);
    }

    SUBCASE("A body leaving the hill when it is not currently on the hill does nothing")
    {
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        end_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 1);

        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 0);

        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        end_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        end_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 0);

        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(hill_occupant_count(registry.get<EcsHill>(hill_entity)) == 1);
    }
}
}