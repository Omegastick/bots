#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

#include "hill_system.h"
#include "environment/components/hill.h"
#include "environment/components/score.h"
#include "environment/utils/hill_utils.h"

namespace ai
{
void hill_system(entt::registry &registry)
{
    registry.view<EcsHill>().each([&](auto &hill) {
        if (hill.occupant_count == 1)
        {
            auto &score = registry.get<Score>(hill.occupants[0]);
            score.score += 1.f;
        }
    });
}

void reset_hill(entt::registry &registry)
{
    registry.view<EcsHill>().each([&](auto &hill) {
        hill.occupant_count = 0;
    });
}

TEST_CASE("Hill system")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto hill_entity = make_hill(registry, {0.f, 0.f}, 1.f);

    const auto body_1 = registry.create();
    registry.emplace<Score>(body_1);
    const auto body_2 = registry.create();
    registry.emplace<Score>(body_2);

    auto &hill = registry.get<EcsHill>(hill_entity);

    SUBCASE("If no bodies are on the hill, nothing happens")
    {
        hill_system(registry);

        DOCTEST_CHECK(registry.get<Score>(body_1).score == 0);
        DOCTEST_CHECK(registry.get<Score>(body_2).score == 0);
    }

    SUBCASE("If one body is on the hill, that body is awarded 1 point")
    {
        hill.occupants[0] = body_1;
        hill.occupant_count = 1;

        hill_system(registry);

        DOCTEST_CHECK(registry.get<Score>(body_1).score == 1);
        DOCTEST_CHECK(registry.get<Score>(body_2).score == 0);
    }

    SUBCASE("If two bodies are on the hill, nothing happens")
    {
        hill.occupants[0] = body_1;
        hill.occupants[1] = body_2;
        hill.occupant_count = 2;

        hill_system(registry);

        DOCTEST_CHECK(registry.get<Score>(body_1).score == 0);
        DOCTEST_CHECK(registry.get<Score>(body_2).score == 0);
    }
}
}