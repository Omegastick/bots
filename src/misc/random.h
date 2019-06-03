#pragma once

#include <random>

namespace SingularityTrainer
{
class Random
{
  private:
    std::mt19937 rng;

  public:
    Random(int seed);
    Random(Random &&other);
    ~Random();

    int next_int(int min, int max);
    float next_float(float min, float max);
    float next_float(std::uniform_real_distribution<float> distribution);
};
}