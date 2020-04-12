#include <memory>
#include <tuple>
#include <vector>

#include <doctest.h>
#include <nlohmann/json.hpp>
#include <torch/torch.h>

#include "evaluator.h"
#include "audio/audio_engine.h"
#include "environment/ecs_env.h"
#include "environment/serialization/serialize_body.h"
#include "misc/random.h"
#include "training/agents/iagent.h"
#include "training/agents/random_agent.h"

namespace ai
{
Evaluator::Evaluator() {}

EvaluationResult Evaluator::evaluate(const IAgent &agent_1, const IAgent &agent_2)
{
    // Initialize environment
    EcsEnv environment;
    environment.set_audibility(false);
    environment.set_body(0, agent_1.get_body_spec());
    environment.set_body(1, agent_2.get_body_spec());

    // Get first observations
    auto observation_1 = torch::zeros({1, agent_1.get_body_spec()["num_observations"]});
    auto observation_2 = torch::zeros({1, agent_2.get_body_spec()["num_observations"]});
    auto step_info = environment.reset();
    observation_1 = step_info.observations[0];
    observation_2 = step_info.observations[1];

    // Initialize masks and hidden states
    auto hidden_state_1 = torch::zeros({1, agent_1.get_hidden_state_size()});
    auto hidden_state_2 = torch::zeros({1, agent_2.get_hidden_state_size()});
    auto mask_1 = torch::ones({1, 1});
    auto mask_2 = torch::ones({1, 1});

    // Run every environment to end
    bool done = false;
    int victor;
    while (!done)
    {
        const auto act_result_1 = agent_1.act(observation_1,
                                              hidden_state_1,
                                              mask_1);
        const auto act_result_2 = agent_2.act(observation_2,
                                              hidden_state_2,
                                              mask_2);
        hidden_state_1 = act_result_1.hidden_state;
        hidden_state_2 = act_result_2.hidden_state;
        auto step_info = environment.step({act_result_1.action, act_result_2.action}, 1.f / 60.f);
        for (int k = 0; k < 5; ++k)
        {
            environment.forward(1.f / 60.f);
        }

        observation_1 = step_info.observations[0];
        observation_2 = step_info.observations[1];

        if (step_info.done[0].item().toBool())
        {
            victor = step_info.victor;
            done = true;
        }
    }

    if (victor == 0)
    {
        return EvaluationResult::Agent1;
    }
    else if (victor == 1)
    {
        return EvaluationResult::Agent2;
    }

    return EvaluationResult::Draw;
}

TEST_CASE("Evaluator")
{
    SUBCASE("evaluate() returns a draw if the length of the game is too short for a win")
    {
        Random rng(0);
        Evaluator evaluator;

        RandomAgent agent(default_body(), rng, "Random Agent");

        auto result = evaluator.evaluate(agent, agent);

        DOCTEST_CHECK(result == EvaluationResult::Draw);
    }
}
}