#pragma once

#include <random>

#include "third_party/di.hpp"

namespace ai
{
static auto RandomSeed = [] {};

class Random
{
  private:
    std::mt19937 rng;

  public:
    BOOST_DI_INJECT(Random, (named = RandomSeed) int seed);
    Random(Random &&other);

    bool next_bool(double probability);
    int next_int(int min, int max);
    float next_float(float min, float max);
    float next_float(std::uniform_real_distribution<float> distribution);
};
}