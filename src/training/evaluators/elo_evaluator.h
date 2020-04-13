#pragma once

#include <memory>
#include <vector>
#include <map>

#include <cpprl/model/policy.h>
#include <nlohmann/json_fwd.hpp>

#include "training/agents/iagent.h"
#include "training/agents/nn_agent.h"
#include "training/evaluators/evaluator.h"

namespace ai
{
class Random;

class EloEvaluator : protected Evaluator
{
  private:
    std::map<const IAgent *, double> elos;
    std::unique_ptr<IAgent> main_agent;
    std::vector<std::unique_ptr<IAgent>> opponents;
    Random &rng;

  public:
    EloEvaluator(Random &rng, double game_length = 60.f);

    double evaluate(IAgent &agent,
                    const std::vector<IAgent *> &new_opponents,
                    unsigned int number_of_trials);
};
}