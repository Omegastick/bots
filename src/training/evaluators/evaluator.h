#pragma once

#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace SingularityTrainer
{
class BodyFactory;
class IAgent;
class IEnvironmentFactory;

enum class EvaluationResult
{
    Draw,
    Agent1,
    Agent2
};

class Evaluator
{
  private:
    BodyFactory &body_factory;
    IEnvironmentFactory &env_factory;

  public:
    Evaluator(BodyFactory &body_factory, IEnvironmentFactory &env_factory);

    std::vector<EvaluationResult> evaluate(const IAgent &agent_1,
                                           const IAgent &agent_2,
                                           int number_of_trials);
};
}
