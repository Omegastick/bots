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

double BasicEvaluator::evaluate(cpprl::Policy policy, nlohmann::json &body_spec, int number_of_trials)
{
    TestBody test_body(rng);
    auto test_body_json = test_body.to_json();
    RandomAgent random_agent(test_body_json, rng);
    NNAgent nn_agent(policy, body_spec);

    auto results = Evaluator::evaluate(nn_agent, random_agent, number_of_trials);

    double total = std::accumulate(results.begin(), results.end(),
                                   0.,
                                   [](double total, EvaluationResult result) {
                                       if (result == EvaluationResult::Agent1)
                                       {
                                           return total + 1;
                                       }
                                       else if (result == EvaluationResult::Draw)
                                       {
                                           return total + 0.5;
                                       } else {
                                         return total;
                                       }
                                   });
    return total / results.size();
}
}