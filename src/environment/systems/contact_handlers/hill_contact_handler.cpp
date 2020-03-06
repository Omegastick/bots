#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>

#include "hill_contact_handler.h"
#include "environment/components/hill.h"
#include "environment/components/physics_type.h"
#include "environment/utils/hill_utils.h"

namespace ai
{
void begin_hill_contact(entt::registry &registry,
                        entt::entity hill_entity,
                        entt::entity other_entity,
                        PhysicsType::Type other_type)
{
    if (other_type != PhysicsType::Body)
    {
        return;
    }

    auto &hill = registry.get<EcsHill>(hill_entity);
    if (std::find(hill.occupants.begin(),
                  hill.occupants.end(),
                  other_entity) == hill.occupants.end())
    {
        hill.occupants[hill.occupant_count] = other_entity;
        hill.occupant_count++;
    }
}

void end_hill_contact(entt::registry &registry,
                      entt::entity hill_entity,
                      entt::entity other_entity,
                      PhysicsType::Type other_type)
{
    if (other_type != PhysicsType::Body)
    {
        return;
    }

    auto &hill = registry.get<EcsHill>(hill_entity);
    if (hill.occupants[1] == other_entity)
    {
        hill.occupant_count--;
    }
    else if (hill.occupants[0] == other_entity)
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
    const auto body_2 = registry.create();

    SUBCASE("Counts up to two bodies on the hill")
    {
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 0);

        begin_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);

        begin_hill_contact(registry, hill_entity, body_2, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 2);
    }

    SUBCASE("Counts down as bodies leave the hill")
    {
        begin_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        begin_hill_contact(registry, hill_entity, body_2, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 2);

        end_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);

        end_hill_contact(registry, hill_entity, body_2, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 0);
    }

    SUBCASE("The same body entering the hill twice does nothing")
    {
        begin_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        begin_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);
    }

    SUBCASE("The same body leaving the hill twice does nothing")
    {
        begin_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        begin_hill_contact(registry, hill_entity, body_2, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 2);

        end_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);

        end_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupant_count == 1);
    }

    SUBCASE("When there is only one body on the hill, the correct entity is in slot 0")
    {
        begin_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupants[0] == body_1);

        begin_hill_contact(registry, hill_entity, body_2, PhysicsType::Body);
        end_hill_contact(registry, hill_entity, body_1, PhysicsType::Body);
        DOCTEST_CHECK(registry.get<EcsHill>(hill_entity).occupants[0] == body_2);
    }
}
}