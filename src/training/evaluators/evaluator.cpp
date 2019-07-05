#include <memory>
#include <tuple>
#include <vector>

#include <doctest.h>
#include <nlohmann/json.hpp>
#include <torch/torch.h>

#include "evaluator.h"
#include "misc/random.h"
#include "training/agents/iagent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"

namespace SingularityTrainer
{
Evaluator::Evaluator(BodyFactory &body_factory,
                     IEnvironmentFactory &env_factory)
    : body_factory(body_factory),
      env_factory(env_factory) {}

std::vector<EvaluationResult> Evaluator::evaluate(const IAgent &agent_1,
                                                  const IAgent &agent_2,
                                                  int number_of_trials)
{
    // Initialize environments
    std::vector<int> scores(number_of_trials);

    std::vector<std::unique_ptr<IEnvironment>> environments(number_of_trials);
    for (int i = 0; i < number_of_trials; ++i)
    {
        auto world = std::make_unique<b2World>(b2Vec2_zero);
        auto rng = std::make_unique<Random>(i);
        std::vector<std::unique_ptr<Body>> bodies;
        bodies.push_back(body_factory.make(*world, *rng));
        bodies.push_back(body_factory.make(*world, *rng));
        bodies[0]->load_json(agent_1.get_body_spec());
        bodies[1]->load_json(agent_2.get_body_spec());
        environments[i] = env_factory.make(std::move(rng),
                                           std::move(world),
                                           std::move(bodies),
                                           RewardConfig());
    }

    // Get first observations
    auto observations_1 = torch::zeros({number_of_trials, agent_1.get_body_spec()["num_observations"]});
    auto observations_2 = torch::zeros({number_of_trials, agent_2.get_body_spec()["num_observations"]});
    for (int i = 0; i < number_of_trials; ++i)
    {
        auto env_observation = environments[i]->reset().observation;
        observations_1[i] = env_observation[0];
        observations_2[i] = env_observation[1];
    }

    // Initialize masks and hidden states
    auto hidden_states_1 = torch::zeros({number_of_trials, agent_1.get_hidden_state_size()});
    auto hidden_states_2 = torch::zeros({number_of_trials, agent_2.get_hidden_state_size()});
    auto masks_1 = torch::ones({number_of_trials, 1});
    auto masks_2 = torch::ones({number_of_trials, 1});

    // Run every environment to end
    std::vector<bool> finished_trials(number_of_trials, false);
    int finished_count = 0;
    while (finished_count < number_of_trials)
    {
        torch::Tensor actions_1;
        torch::Tensor actions_2;
        std::tie(actions_1, hidden_states_1) = agent_1.act(observations_1,
                                                           hidden_states_1,
                                                           masks_1);
        std::tie(actions_2, hidden_states_2) = agent_2.act(observations_2,
                                                           hidden_states_2,
                                                           masks_2);
        torch::Tensor dones = torch::zeros({number_of_trials, 1});
        std::vector<StepInfo> step_infos(number_of_trials);
        int j = 0;
        for (int i = 0; i < number_of_trials; ++i)
        {
            if (!finished_trials[i])
            {
                step_infos[i] = environments[i]->step({actions_1[j],
                                                       actions_2[j]},
                                                      1. / 60.);
                j++;
            }
        }
        auto observations_1 = torch::zeros({number_of_trials - finished_count, agent_1.get_body_spec()["num_observations"]});
        auto observations_2 = torch::zeros({number_of_trials - finished_count, agent_2.get_body_spec()["num_observations"]});
        j = 0;
        for (int i = 0; i < number_of_trials; ++i)
        {
            if (!finished_trials[i])
            {
                auto &step_info = step_infos[i];
                observations_1[j] = step_info.observation[0];
                observations_2[j] = step_info.observation[1];

                if (step_info.done[0].item().toBool())
                {
                    scores[i] = step_info.victor;
                    finished_trials[i] = true;
                    finished_count++;
                }

                j++;
            }
        }
    }

    std::vector<EvaluationResult> results;
    results.reserve(number_of_trials);
    for (const auto &score : scores)
    {
        if (score == 0)
        {
            results.push_back(EvaluationResult::Agent1);
        }
        else if (score == 1)
        {
            results.push_back(EvaluationResult::Agent2);
        }
        else
        {
            results.push_back(EvaluationResult::Draw);
        }
    }

    return results;
}

TEST_CASE("Evaluator")
{
    SUBCASE("evaluate() runs the correct number of trials")
    {
        KothEnvFactory env_factory(100);
        Random rng(0);
        BodyFactory body_factory(rng);
        Evaluator evaluator(body_factory, env_factory);

        TestBody body(rng);
        auto body_spec = body.to_json();

        RandomAgent agent(body_spec, rng);

        auto results = evaluator.evaluate(agent,
                                          agent,
                                          4);

        DOCTEST_CHECK(results.size() == 4);
    }
}
}