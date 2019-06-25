#include <doctest.h>

#include "random_agent.h"
#include "misc/random.h"

namespace SingularityTrainer
{
RandomAgent::RandomAgent(int num_outputs, Random &rng)
    : num_outputs(num_outputs), rng(rng) {}

std::vector<int> RandomAgent::act(torch::Tensor observation,
                                  torch::Tensor hidden_state,
                                  torch::Tensor mask)
{
    std::vector<int> actions(num_outputs);
    for (int i = 0; i < num_outputs; ++i)
    {
        actions[i] = rng.next_int(0, 2);
    }
    return actions;
}

std::vector<std::vector<int>> RandomAgent::act_multiple(torch::Tensor observations,
                                                        torch::Tensor hidden_states,
                                                        torch::Tensor masks)
{
    int num_bodies = observations.size(0);
    std::vector<std::vector<int>> actions(num_bodies);
    for (int i = 0; i < num_bodies; ++i)
    {
        actions[i].resize(num_outputs);
        for (int j = 0; j < num_outputs; ++j)
        {
            actions[i][j] = rng.next_int(0, 2);
        }
    }
    return actions;
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

            DOCTEST_CHECK(actions.size() == 4);
        }

        SUBCASE("With [1, N] shaped tensor")
        {
            auto actions = agent.act(torch::zeros({1, 5}),
                                     torch::zeros({1, 6}),
                                     torch::zeros({1, 1}));

            DOCTEST_CHECK(actions.size() == 4);
        }
    }

    SUBCASE("act_multiple() returns correctly sized actions")
    {
        SUBCASE("Dimension 1")
        {
            auto actions = agent.act_multiple(torch::zeros({3, 5}),
                                              torch::zeros({3, 6}),
                                              torch::zeros({3, 1}));

            DOCTEST_CHECK(actions[0].size() == 4);
            DOCTEST_CHECK(actions[1].size() == 4);
            DOCTEST_CHECK(actions[2].size() == 4);
        }

        SUBCASE("Dimension 0")
        {
            auto actions = agent.act_multiple(torch::zeros({3, 5}),
                                              torch::zeros({3, 6}),
                                              torch::zeros({3, 1}));

            DOCTEST_CHECK(actions.size() == 3);
        }
    }
}
}