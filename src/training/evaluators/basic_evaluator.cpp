#include <cpprl/model/policy.h>
#include <nlohmann/json.hpp>

#include "basic_evaluator.h"
#include "training/agents/nn_agent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/test_body.h"

namespace SingularityTrainer
{
BasicEvaluator::BasicEvaluator(BodyFactory &body_factory,
                               IEnvironmentFactory &env_factory,
                               Random &rng)
    : Evaluator(body_factory, env_factory),
      rng(rng) {}

EvaluationResult BasicEvaluator::evaluate(cpprl::Policy policy, nlohmann::json &body_spec, int number_of_trials)
{
    TestBody test_body(rng);
    auto test_body_json = test_body.to_json();
    RandomAgent random_agent(test_body.get_input_count(), rng);
    NNAgent nn_agent(policy);
    return Evaluator::evaluate(nn_agent, random_agent, body_spec, test_body_json, number_of_trials);
}
}