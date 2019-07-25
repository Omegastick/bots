#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <cpprl/model/policy.h>
#include <nlohmann/json_fwd.hpp>

#include "training/agents/iagent.h"
#include "training/agents/nn_agent.h"
#include "training/evaluators/evaluator.h"

namespace SingularityTrainer
{
class Random;

class EloEvaluator : protected Evaluator
{
  private:
    std::unordered_map<const IAgent *, double> elos;
    std::unique_ptr<IAgent> main_agent;
    std::vector<std::unique_ptr<IAgent>> opponents;
    Random &rng;

  public:
    EloEvaluator(BodyFactory &body_factory, IEnvironmentFactory &env_factory, Random &rng);

    double evaluate(IAgent &agent, const std::vector<IAgent *> &new_opponents);
};
}