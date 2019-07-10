#pragma once

#include <tuple>
#include <vector>

#include <torch/torch.h>

namespace SingularityTrainer
{
typedef std::tuple<std::vector<float>, std::vector<float>> MeanVar;

// https://github.com/openai/baselines/blob/master/baselines/common/running_mean_std.py
class RunningMeanStd
{
  private:
    float count;
    torch::Tensor mean, variance;

    void update_from_moments(torch::Tensor batch_mean,
                             torch::Tensor batch_var,
                             torch::Tensor batch_count);

  public:
    explicit RunningMeanStd(int size);
    RunningMeanStd(std::vector<float> means, std::vector<float> variances);

    void update(torch::Tensor observation);

    inline torch::Tensor get_mean() { return mean.clone(); }
    inline torch::Tensor get_variance() { return variance.clone(); }
};

class ObservationNormalizer
{
  private:
    float clip;
    RunningMeanStd rms;
    bool training;

  public:
    explicit ObservationNormalizer(int size, float clip = 10.);
    ObservationNormalizer(std::vector<float> means,
                          std::vector<float> variances,
                          float clip = 10.);

    torch::Tensor process_observation(torch::Tensor observation);
    MeanVar get_mean_and_var() const;
};
}