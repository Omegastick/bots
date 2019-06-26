#include <doctest.h>
#include <cpprl/cpprl.h>

#include "nn_agent.h"

namespace SingularityTrainer
{
NNAgent::NNAgent(cpprl::Policy policy)
    : policy(policy) {}

ActResult NNAgent::act(torch::Tensor observations,
                       torch::Tensor hidden_states,
                       torch::Tensor masks)
{
    if (observations.dim() == 1)
    {
        observations = observations.unsqueeze(0);
        hidden_states = hidden_states.unsqueeze(0);
        masks = masks.unsqueeze(0);
    }
    auto act_result = policy->act(observations,
                                  hidden_states,
                                  masks);
    return {act_result[1], act_result[3]};
}
TEST_CASE("NNAgent")
{
    auto nn_base = std::make_shared<cpprl::MlpBase>(5, true, 6);
    auto policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {4}}, nn_base);
    NNAgent agent(policy);

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