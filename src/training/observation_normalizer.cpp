#include <doctest.h>
#include <torch/torch.h>

#include "observation_normalizer.h"

namespace SingularityTrainer
{
RunningMeanStd::RunningMeanStd(int size)
    : count(1e-4),
      mean(torch::zeros({size})),
      variance(torch::zeros({size})) {}

RunningMeanStd::RunningMeanStd(std::vector<float> means, std::vector<float> variances)
    : count(1e-4),
      mean(torch::zeros({1})),
      variance(torch::zeros({1})) {}

void RunningMeanStd::update(torch::Tensor observation) {}
void RunningMeanStd::update_from_moments(torch::Tensor batch_mean,
                                         torch::Tensor batch_var,
                                         torch::Tensor batch_count) {}

ObservationNormalizer::ObservationNormalizer(int size, float clip)
    : clip(clip),
      rms(size),
      training(true) {}
ObservationNormalizer::ObservationNormalizer(std::vector<float> means,
                                             std::vector<float> variances,
                                             float clip)
    : clip(clip),
      rms(means, variances),
      training(true) {}

torch::Tensor ObservationNormalizer::process_observation(torch::Tensor observation) {}
MeanVar ObservationNormalizer::get_mean_and_var() const {}

TEST_CASE("RunningMeanStd")
{
    SUBCASE("Correctly calculates mean and variance")
    {
        RunningMeanStd rms(5);
        auto observations = torch::rand({3, 5});
        rms.update(observations[0]);
        rms.update(observations[1]);
        rms.update(observations[2]);

        auto expected_mean = observations.mean(0);
        auto expected_variance = observations.std(0);

        auto actual_mean = rms.get_mean();
        auto actual_variance = rms.get_variance();

        for (int i = 0; i < 5; ++i)
        {
            DOCTEST_CHECK(expected_mean[i].item().toFloat() ==
                          doctest::Approx(actual_mean[i].item().toFloat()));
            DOCTEST_CHECK(expected_variance[i].item().toFloat() ==
                          doctest::Approx(actual_variance[i].item().toFloat()));
        }
    }
}

TEST_CASE("ObservationNormalizer")
{
    SUBCASE("Clips values correctly")
    {
        ObservationNormalizer normalizer(7, 1);
        float observation_array[] = {-1000, -100, -10, 0, 10, 100, 1000};
        auto observation = torch::from_blob(observation_array, {7});
        auto processed_observation = normalizer.process_observation(observation);

        auto has_too_large_values = (processed_observation > 1).any().item().toBool();
        auto has_too_small_values = (processed_observation < 1).any().item().toBool();
        DOCTEST_CHECK(!has_too_large_values);
        DOCTEST_CHECK(!has_too_small_values);
    }

    SUBCASE("Normalizes values correctly")
    {
        ObservationNormalizer normalizer(5);

        float obs_1_array[] = {-10., 0., 5., 3.2, 0.};
        float obs_2_array[] = {-5., 2., 4., 3.7, -3.};
        float obs_3_array[] = {1, 2, 3, 4, 5};
        auto obs_1 = torch::from_blob(obs_1_array, {5});
        auto obs_2 = torch::from_blob(obs_2_array, {5});
        auto obs_3 = torch::from_blob(obs_3_array, {5});

        normalizer.process_observation(obs_1);
        normalizer.process_observation(obs_2);
        auto processed_observation = normalizer.process_observation(obs_1);

        DOCTEST_CHECK(processed_observation[0].item().toFloat() == doctest::Approx(3.39915672));
        DOCTEST_CHECK(processed_observation[1].item().toFloat() == doctest::Approx(1.00002499));
        DOCTEST_CHECK(processed_observation[2].item().toFloat() == doctest::Approx(-2.9932713));
        DOCTEST_CHECK(processed_observation[3].item().toFloat() == doctest::Approx(2.18947446));
        DOCTEST_CHECK(processed_observation[4].item().toFloat() == doctest::Approx(0.23080011));
    }
}
}