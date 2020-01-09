#include <doctest.h>
#include <cpprl/cpprl.h>
#include <nlohmann/json.hpp>

#include "nn_agent.h"
#include "audio/audio_engine.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"

namespace ai
{
NNAgent::NNAgent(cpprl::Policy policy, const nlohmann::json &body_spec, const std::string &name)
    : IAgent(body_spec, name),
      policy(policy) {}

ActResult NNAgent::act(torch::Tensor observations,
                       torch::Tensor hidden_states,
                       torch::Tensor masks) const
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
    return {act_result[0], act_result[1], act_result[2], act_result[3]};
}

std::unique_ptr<IAgent> NNAgent::clone() const
{
    return std::make_unique<NNAgent>(policy, body_spec, name);
}

TEST_CASE("NNAgent")
{
    auto nn_base = std::make_shared<cpprl::MlpBase>(5, true, 6);
    auto policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {4}}, nn_base);
    Random rng(0);
    MockAudioEngine audio_engine;
    BulletFactory bullet_factory(audio_engine);
    ModuleFactory module_factory(bullet_factory, rng);
    TestBody body(module_factory, rng);
    NNAgent agent(policy, body.to_json(), "Test");

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