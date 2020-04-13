#include <doctest.h>
#include <nlohmann/json.hpp>

#include "random_agent.h"
#include "audio/audio_engine.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"

namespace ai
{
RandomAgent::RandomAgent(const nlohmann::json &body_spec, Random &rng, const std::string &name)
    : IAgent(body_spec, name),
      num_outputs(body_spec["num_actions"]),
      rng(rng) {}

ActResult RandomAgent::act(torch::Tensor observations,
                           torch::Tensor /*hidden_states*/,
                           torch::Tensor /*masks*/) const
{
    if (observations.dim() == 1)
    {
        return {torch::zeros({1}),
                torch::zeros({1, num_outputs}, torch::kInt16).random_(2),
                torch::zeros({1}),
                torch::zeros({1})};
    }
    return {torch::zeros({1}),
            torch::zeros({observations.size(0), num_outputs}, torch::kInt16).random_(2),
            torch::zeros({1}),
            torch::zeros({1})};
}

std::unique_ptr<IAgent> RandomAgent::clone() const
{
    return std::make_unique<RandomAgent>(body_spec, rng, name);
}

TEST_CASE("RandomAgent")
{
    Random rng(0);
    MockAudioEngine audio_engine;
    BulletFactory bullet_factory(audio_engine);
    ModuleFactory module_factory(audio_engine, bullet_factory, rng);
    TestBody body(module_factory, rng);
    RandomAgent agent(body.to_json(), rng, "Test");

    SUBCASE("act() returns correctly sized actions")
    {
        SUBCASE("With [N] shaped tensor")
        {
            auto actions = agent.act(torch::zeros({5}),
                                     torch::zeros({6}),
                                     torch::zeros({1}));

            DOCTEST_CHECK(actions.action.size(0) == 1);
            DOCTEST_CHECK(actions.action.size(1) == 4);
        }

        SUBCASE("With [1, N] shaped tensor")
        {
            auto actions = agent.act(torch::zeros({1, 5}),
                                     torch::zeros({1, 6}),
                                     torch::zeros({1, 1}));

            DOCTEST_CHECK(actions.action.size(0) == 1);
            DOCTEST_CHECK(actions.action.size(1) == 4);
        }

        SUBCASE("Multiple parallel actions")
        {
            auto actions = agent.act(torch::zeros({3, 5}),
                                     torch::zeros({3, 6}),
                                     torch::zeros({3, 1}));

            DOCTEST_CHECK(actions.action.size(0) == 3);
            DOCTEST_CHECK(actions.action.size(1) == 4);
        }
    }
}
}