#include <random>

#include <doctest.h>

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

TEST_CASE("Random")
{
    Random rng(1);

    SUBCASE("Generated floats should be in the right range")
    {
        for (int i = 0; i < 20; ++i)
        {
            auto number = rng.next_float(-10, 10);
            CHECK(number >= -10);
            CHECK(number <= 10);
        }
    }

    SUBCASE("Generated ints should be in the right range")
    {
        for (int i = 0; i < 20; ++i)
        {
            auto number = rng.next_int(-10, 10);
            CHECK(number >= -10);
            CHECK(number <= 10);
        }
    }
}
}