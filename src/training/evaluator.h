#pragma once

#include <nlohmann/json_fwd.hpp>

namespace SingularityTrainer
{
class BodyFactory;
class IAgent;
class IEnvironmentFactory;

struct EvaluationResult
{
    int agent_1;
    int agent_2;
    int draw;
};

class Evaluator
{
  private:
  BodyFactory &body_factory;
    IEnvironmentFactory &env_factory;

  public:
    Evaluator(BodyFactory &body_factory, IEnvironmentFactory &env_factory);

    EvaluationResult evaluate(IAgent &agent_1,
                              IAgent &agent_2,
                              const nlohmann::json &body_1_spec,
                              const nlohmann::json &body_2_spec,
                              int number_of_trials);
};
}
