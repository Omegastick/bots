#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

#include "hill_system.h"
#include "environment/components/body.h"
#include "environment/components/hill.h"
#include "environment/components/score.h"
#include "environment/utils/hill_utils.h"
#include "training/training_program.h"

namespace ai
{
void hill_system(entt::registry &registry)
{
    registry.view<EcsHill>().each([&](auto &hill) {
        if (hill_occupant_count(hill) == 1)
        {
            auto &score = registry.get<Score>(hill.occupants[0].first);
            score.score += 1.f;

            const auto *reward_config = registry.try_ctx<RewardConfig>();
            if (reward_config)
            {
                const auto body_view = registry.view<EcsBody>();
                for (const auto body : body_view)
                {
                    auto &score = registry.get<Score>(hill.occupants[0].first);
                    if (body == hill.occupants[0].first)
                    {
                        score.score += reward_config->hill_tick_reward;
                    }
                    else
                    {
                        score.score += reward_config->enemy_hill_tick_punishment;
                    }
                }
            }
        }
    });
}

void reset_hill(entt::registry &registry)
{
    registry.view<EcsHill>().each([&](auto &hill) {
        hill.occupants = {std::pair<entt::entity, unsigned int>{entt::null, 0},
                          std::pair<entt::entity, unsigned int>{entt::null, 0}};
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
        hill.occupants[0] = {body_1, 1};

        hill_system(registry);

        DOCTEST_CHECK(registry.get<Score>(body_1).score == 1);
        DOCTEST_CHECK(registry.get<Score>(body_2).score == 0);
    }

    SUBCASE("If two bodies are on the hill, nothing happens")
    {
        hill.occupants[0] = {body_1, 1};
        hill.occupants[1] = {body_2, 1};

        hill_system(registry);

        DOCTEST_CHECK(registry.get<Score>(body_1).score == 0);
        DOCTEST_CHECK(registry.get<Score>(body_2).score == 0);
    }
}
}