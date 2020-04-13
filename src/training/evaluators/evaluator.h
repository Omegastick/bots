#pragma once

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
  private:
    double game_length;

  public:
    Evaluator(double game_length = 60.f);

    EvaluationResult evaluate(const IAgent &agent_1, const IAgent &agent_2);
};
}
