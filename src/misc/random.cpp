#include <random>

#include <doctest.h>

#include "misc/random.h"

namespace ai
{
Random::Random(int seed) : rng(seed) {}

Random::Random(Random &&other) : rng(std::move(other.rng)) {}

bool Random::next_bool(double probability)
{
    return next_float(0, 1) < probability;
}

int Random::next_int(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max - 1);
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
        bool out_of_range = false;
        for (int i = 0; i < 20; ++i)
        {
            auto number = rng.next_float(-10, 10);
            out_of_range = out_of_range || number < -10 || number > 10;
        }
        DOCTEST_CHECK(!out_of_range);
    }

    SUBCASE("Generated ints should be in the right range")
    {
        bool out_of_range = false;
        for (int i = 0; i < 20; ++i)
        {
            auto number = rng.next_int(-10, 10);
            out_of_range = out_of_range || number < -10 || number >= 10;
        }
        DOCTEST_CHECK(!out_of_range);
    }
}
}