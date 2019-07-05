#pragma once

#include <cpprl/model/policy.h>
#include <nlohmann/json_fwd.hpp>

#include "training/evaluators/evaluator.h"

namespace SingularityTrainer
{
class BodyFactory;
class IEnvironmentFactory;
class Random;

class BasicEvaluator : protected Evaluator
{
  private:
    Random &rng;

  public:
    BasicEvaluator(BodyFactory &body_factory, IEnvironmentFactory &env_factory, Random &rng);

    double evaluate(cpprl::Policy policy, nlohmann::json &body_spec, int number_of_trials);
};
}