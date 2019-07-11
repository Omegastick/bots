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

    inline int get_count() const { return static_cast<int>(count); }
    inline torch::Tensor get_mean() const { return mean.clone(); }
    inline torch::Tensor get_variance() const { return variance.clone(); }
    inline void set_count(int count) { this->count = count + 1e-8; }
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
    explicit ObservationNormalizer(const std::vector<ObservationNormalizer> &others);

    torch::Tensor process_observation(torch::Tensor observation);
    std::vector<float> get_mean() const;
    std::vector<float> get_variance() const;

    inline void evaluate() { training = false; }
    inline float get_clip_value() const { return clip; }
    inline int get_step_count() const { return rms.get_count(); }
    inline void train() { training = true; }
};
}