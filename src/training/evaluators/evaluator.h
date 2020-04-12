#pragma once

#include <nlohmann/json_fwd.hpp>

namespace ai
{
class IAgent;

enum class EvaluationResult
{
    Draw,
    Agent1,
    Agent2
};

class Evaluator
{
  public:
    Evaluator();

    EvaluationResult evaluate(const IAgent &agent_1, const IAgent &agent_2);
};
}
