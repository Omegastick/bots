#include <vector>

#include "training/score_processor.h"

namespace SingularityTrainer
{
ScoreProcessor::ScoreProcessor(int agent_count, float smoothing_weight) : smoothing_weight(smoothing_weight)
{
    moving_averages.resize(agent_count);
}

void ScoreProcessor::add_score(int agent, float score)
{
    moving_averages[agent] = moving_averages[agent] * smoothing_weight + (1 - smoothing_weight) * score;
}
}