#pragma once

#include <nlohmann/json_fwd.hpp>

#include "training/evaluators/evaluator.h"

namespace ai
{
class BodyFactory;
class IAgent;
class IEnvironmentFactory;
class Random;

class BasicEvaluator : protected Evaluator
{
  private:
    Random &rng;

  public:
    BasicEvaluator(Random &rng, double game_length = 60.f);

    double evaluate(const IAgent &agent, int number_of_trials);
};
}