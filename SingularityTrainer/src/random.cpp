#include <random>

#include "random.h"

namespace SingularityTrainer
{
Random::Random(int seed) : rng(seed) {}

Random::~Random() {}

int Random::next_int(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(rng);
}

float Random::next_float(float min, float max)
{
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(rng);
}

float Random::next_float(std::uniform_real_distribution<float> distribution)
{
    return distribution(rng);
}
}