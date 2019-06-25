#include <doctest.h>
#include <cpprl/cpprl.h>

#include "nn_agent.h"

namespace SingularityTrainer
{
NNAgent::NNAgent(cpprl::Policy policy)
    : policy(policy) {}

std::vector<int> NNAgent::act(torch::Tensor observation,
                              torch::Tensor hidden_state,
                              torch::Tensor mask)
{
    auto act_result = policy->act(observation.reshape({1, -1}),
                                  hidden_state.reshape({1, -1}),
                                  mask.reshape({1, -1}));
    auto actions_tensor = act_result[1].to(torch::kInt).contiguous();
    return std::vector<int>(actions_tensor.data<int>(),
                            actions_tensor.data<int>() + actions_tensor.numel());
}

std::vector<std::vector<int>> NNAgent::act_multiple(torch::Tensor observations,
                                                    torch::Tensor hidden_states,
                                                    torch::Tensor masks)
{
    auto act_result = policy->act(observations, hidden_states, masks);
    auto actions_tensor = act_result[1].to(torch::kInt).contiguous();
    int num_bodies = observations.size(0);
    std::vector<std::vector<int>> actions(num_bodies);
    for (int i = 0; i < num_bodies; ++i)
    {
        actions[i] = std::vector<int>(actions_tensor[i].data<int>(),
                                      actions_tensor[i].data<int>() + actions_tensor[i].numel());
    }
    return actions;
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