#pragma once

#include <vector>

namespace ai
{
class ScoreProcessor
{
  private:
    std::vector<float> moving_averages;
    float smoothing_weight;

  public:
    ScoreProcessor(int agent_count, float smoothing_weight);

    void add_score(int agent, float score);

    inline const std::vector<float> &get_scores() const { return moving_averages; }
};
}