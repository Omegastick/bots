#include <doctest.h>
#include <torch/torch.h>

#include "observation_normalizer.h"

namespace SingularityTrainer
{
RunningMeanStd::RunningMeanStd(int size)
    : count(1e-4),
      mean(torch::zeros({size})),
      variance(torch::ones({size})) {}

RunningMeanStd::RunningMeanStd(std::vector<float> means, std::vector<float> variances)
    : count(1e-4),
      mean(torch::from_blob(means.data(), {static_cast<long>(means.size())})),
      variance(torch::from_blob(variances.data(), {static_cast<long>(variances.size())})) {}

void RunningMeanStd::update(torch::Tensor observation)
{
    observation = observation.reshape({-1, mean.size(0)});
    auto batch_mean = observation.mean(0);
    auto batch_var = observation.var(0, false, false);
    auto batch_count = observation.size(0);

    update_from_moments(batch_mean, batch_var, batch_count);
}

void RunningMeanStd::update_from_moments(torch::Tensor batch_mean,
                                         torch::Tensor batch_var,
                                         int batch_count)
{
    auto delta = batch_mean - mean;
    float total_count = count + batch_count;

    mean = mean + delta * batch_count / total_count;
    auto m_a = variance * count;
    auto m_b = batch_var * batch_count;
    auto m2 = m_a + m_b + torch::pow(delta, 2) * count * batch_count / total_count;
    variance = m2 / total_count;
    count = total_count;
}

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
        auto expected_variance = observations.var(0, false, false);

        auto actual_mean = rms.get_mean();
        auto actual_variance = rms.get_variance();

        for (int i = 0; i < 5; ++i)
        {
            DOCTEST_CHECK(expected_mean[i].item().toFloat() ==
                          doctest::Approx(actual_mean[i].item().toFloat())
                              .epsilon(0.001));
            DOCTEST_CHECK(expected_variance[i].item().toFloat() ==
                          doctest::Approx(actual_variance[i].item().toFloat())
                              .epsilon(0.001));
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