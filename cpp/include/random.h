#pragma once

#include <random>

namespace SingularityTrainer
{
class Random
{
  public:
    Random(int seed);
    ~Random();

    int NextInt(int min, int max);
    float NextFloat(float min, float max);

  private:
    std::mt19937 rng;
};
}