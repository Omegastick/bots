#include <doctest.h>
#include <nlohmann/json.hpp>

#include "random_agent.h"
#include "misc/random.h"
#include "training/bodies/test_body.h"

namespace SingularityTrainer
{
RandomAgent::RandomAgent(const nlohmann::json &body_spec, Random &rng)
    : IAgent(body_spec),
      num_outputs(body_spec["num_actions"]),
      rng(rng) {}

ActResult RandomAgent::act(torch::Tensor observations,
                           torch::Tensor /*hidden_states*/,
                           torch::Tensor /*masks*/)
{
    if (observations.dim() == 1)
    {
        return {torch::zeros({1, num_outputs}).random_(2), {}};
    }
    else
    {
        return {torch::zeros({observations.size(0), num_outputs}).random_(2), {}};
    }
}

TEST_CASE("RandomAgent")
{
    Random rng(0);
    TestBody body(rng);
    RandomAgent agent(body.to_json(), rng);

    SUBCASE("act() returns correctly sized actions")
    {
        SUBCASE("With [N] shaped tensor")
        {
            auto actions = agent.act(torch::zeros({5}),
                                     torch::zeros({6}),
                                     torch::zeros({1}));

            DOCTEST_CHECK(std::get<0>(actions).size(0) == 1);
            DOCTEST_CHECK(std::get<0>(actions).size(1) == 4);
        }

        SUBCASE("With [1, N] shaped tensor")
        {
            auto actions = agent.act(torch::zeros({1, 5}),
                                     torch::zeros({1, 6}),
                                     torch::zeros({1, 1}));

            DOCTEST_CHECK(std::get<0>(actions).size(0) == 1);
            DOCTEST_CHECK(std::get<0>(actions).size(1) == 4);
        }

        SUBCASE("Multiple parallel actions")
        {
            auto actions = agent.act(torch::zeros({3, 5}),
                                     torch::zeros({3, 6}),
                                     torch::zeros({3, 1}));

            DOCTEST_CHECK(std::get<0>(actions).size(0) == 3);
            DOCTEST_CHECK(std::get<0>(actions).size(1) == 4);
        }
    }
}
}