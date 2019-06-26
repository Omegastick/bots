#include <doctest.h>

#include "random_agent.h"
#include "misc/random.h"

namespace SingularityTrainer
{
RandomAgent::RandomAgent(int num_outputs, Random &rng)
    : num_outputs(num_outputs), rng(rng) {}

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
    RandomAgent agent(4, rng);

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