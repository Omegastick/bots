#pragma once

#include <vector>

#include <torch/torch.h>

namespace SingularityTrainer
{
// https://github.com/openai/baselines/blob/master/baselines/common/running_mean_std.py
class RunningMeanStd
{
  private:
    float count;
    torch::Tensor mean, variance;

    void update_from_moments(torch::Tensor batch_mean,
                             torch::Tensor batch_var,
                             int batch_count);

  public:
    explicit RunningMeanStd(int size);
    RunningMeanStd(std::vector<float> means, std::vector<float> variances);

    void update(torch::Tensor observation);

    inline torch::Tensor get_mean() const { return mean.clone(); }
    inline torch::Tensor get_variance() const { return variance.clone(); }
};

class ObservationNormalizer
{
  private:
    float clip;
    RunningMeanStd rms;
    bool training;

  public:
    explicit ObservationNormalizer(int size, float clip = 10.);
    ObservationNormalizer(const std::vector<float> &means,
                          const std::vector<float> &variances,
                          float clip = 10.);

    torch::Tensor process_observation(torch::Tensor observation);
    std::vector<float> get_mean() const;
    std::vector<float> get_variance() const;

    inline void train() { training = true; }
    inline void evaluate() { training = false; }
};
}