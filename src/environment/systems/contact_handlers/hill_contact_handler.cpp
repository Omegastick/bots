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
    if (std::find(hill.occupants.begin(),
                  hill.occupants.end(),
                  body) == hill.occupants.end())
    {
        hill.occupants[hill.occupant_count] = body;
        hill.occupant_count++;
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
    if (hill.occupants[1] == body)
    {
        hill.occupant_count--;
    }
    else if (hill.occupants[0] == body)
    {
        hill.occupants[0] = hill.occupants[1];
        hill.occupant_count--;
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
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 0);

        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);

        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 2);
    }

    SUBCASE("Counts down as bodies leave the hill")
    {
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 2);

        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);

        end_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 0);
    }

    SUBCASE("The same body entering the hill twice does nothing")
    {
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);
    }

    SUBCASE("The same body leaving the hill twice does nothing")
    {
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 2);

        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);

        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);
    }

    SUBCASE("When there is only one body on the hill, the correct entity is in slot 0")
    {
        begin_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupants[0] == body_1);

        begin_hill_contact(registry, hill_entity, module_2, PhysicsType::Module);
        end_hill_contact(registry, hill_entity, module_1, PhysicsType::Module);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupants[0] == body_2);
    }
}
}