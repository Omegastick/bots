#include <doctest.h>
#include <entt/entt.hpp>

#include "environment/components/body.h"
#include "environment/components/done.h"
#include "environment/components/score.h"

namespace ai
{
void body_death_system(entt::registry &registry, entt::entity *bodies, std::size_t body_count)
{
    for (unsigned int i = 0; i < body_count; i++)
    {
        if (registry.get<EcsBody>(bodies[i]).hp <= 0.f)
        {
            for (unsigned int j = 0; j < body_count; j++)
            {
                if (j != i)
                {
                    registry.get<Score>(bodies[j]).score += 100.f;
                }
            }
            registry.ctx<Done>().done = true;
        }
    }
}

TEST_CASE("Body death system")
{
    entt::registry registry;
    registry.set<Done>();

    entt::entity entities[2] = {registry.create(), registry.create()};
    registry.emplace<EcsBody>(entities[0]);
    registry.emplace<EcsBody>(entities[1]);
    registry.emplace<Score>(entities[0]);
    registry.emplace<Score>(entities[1]);

    SUBCASE("If a body has 0 HP, the other body gets 100 points")
    {
        registry.get<EcsBody>(entities[1]).hp = 1;
        registry.get<EcsBody>(entities[0]).hp = 0;

        body_death_system(registry, entities, 2);

        DOCTEST_CHECK(registry.get<Score>(entities[0]).score == 0.f);
        DOCTEST_CHECK(registry.get<Score>(entities[1]).score == 100.f);
    }
}
}