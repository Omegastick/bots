#include <map>
#include <sstream>
#include <vector>

#include <doctest.h>
#include <cpprl/model/policy.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "elo_evaluator.h"
#include "training/agents/iagent.h"
#include "training/agents/nn_agent.h"
#include "training/bodies/body.h"

namespace SingularityTrainer
{
const int number_of_trials = 10;
const double elo_constant = 16;

double expected_win_chance(double a_rating, double b_rating)
{
    return 1. / (1. + pow(10., (b_rating - a_rating) / 400.));
}

std::tuple<double, double> calculate_elos(float a_rating, float b_rating, double k, EvaluationResult result)
{

    float a_win_chance = expected_win_chance(a_rating, b_rating);
    float b_win_chance = expected_win_chance(b_rating, a_rating);

    if (result == EvaluationResult::Agent1)
    {
        a_rating = a_rating + k * (1 - a_win_chance);
        b_rating = b_rating + k * (0 - b_win_chance);
    }
    else if (result == EvaluationResult::Agent2)
    {
        a_rating = a_rating + k * (0 - a_win_chance);
        b_rating = b_rating + k * (1 - b_win_chance);
    }
    else
    {
        a_rating = a_rating + k * (0.5 - a_win_chance);
        b_rating = b_rating + k * (0.5 - b_win_chance);
    }

    return {a_rating, b_rating};
}

EloEvaluator::EloEvaluator(BodyFactory &body_factory,
                           IEnvironmentFactory &env_factory,
                           Random &rng)
    : Evaluator(body_factory, env_factory),
      rng(rng)
{
}

double EloEvaluator::evaluate(cpprl::Policy policy,
                              nlohmann::json &body_spec,
                              const std::vector<IAgent *> &new_opponents)
{
    // Initialize main agent
    if (main_agent == nullptr)
    {
        main_agent = std::make_unique<NNAgent>(policy, body_spec);
        elos[main_agent.get()] = 1500;
    }
    else
    {
        main_agent->set_policy(policy);
    }

    // Add new opponents to opponent pool
    for (const auto &opponent : new_opponents)
    {
        opponents.push_back(opponent->clone());
        elos[opponents.back().get()] = 1500;
    }

    // Calculate new opponent elos
    for (unsigned int i = opponents.size() - new_opponents.size(); i < opponents.size(); ++i)
    {
        // Run new opponent against current pool
        std::vector<std::vector<EvaluationResult>> results;
        for (unsigned int j = 0; j < i; ++j)
        {
            results.push_back(Evaluator::evaluate(*opponents[i], *opponents[j], number_of_trials));
        }

        // Calculate elos
        for (int j = 0; j < number_of_trials; ++j)
        {
            for (unsigned int k = 0; k < results.size(); ++k)
            {
                std::tie(elos[opponents[i].get()],
                         elos[opponents[k].get()]) = calculate_elos(elos[opponents[i].get()],
                                                                    elos[opponents[k].get()],
                                                                    elo_constant,
                                                                    results[k][j]);
            }
        }
    }

    // Run main agent against current pool
    std::vector<std::vector<EvaluationResult>> results;
    for (const auto &opponent : opponents)
    {
        results.push_back(Evaluator::evaluate(*main_agent, *opponent, number_of_trials));
    }

    // Calculate Elo score for main agent
    for (int i = 0; i < number_of_trials; ++i)
    {
        for (unsigned int j = 0; j < results.size(); ++j)
        {
            std::tie(elos[main_agent.get()],
                     elos[opponents[j].get()]) = calculate_elos(elos[main_agent.get()],
                                                                elos[opponents[j].get()],
                                                                elo_constant,
                                                                results[j][i]);
        }
    }

    return elos[main_agent.get()];
}

TEST_CASE("EloEvaluator")
{
    SUBCASE("expected_win_chance()")
    {
        DOCTEST_CHECK(expected_win_chance(1500, 1500) == doctest::Approx(0.5));
        DOCTEST_CHECK(expected_win_chance(300, 300) == doctest::Approx(0.5));
        DOCTEST_CHECK(expected_win_chance(2000.123, 2000.123) == doctest::Approx(0.5));

        DOCTEST_CHECK(expected_win_chance(-1000, 3000) == doctest::Approx(0.0));
        DOCTEST_CHECK(expected_win_chance(3000, -1000) == doctest::Approx(1.0));
    }

    SUBCASE("calculate_elos()")
    {
        auto result = calculate_elos(3000, -1000, 32, EvaluationResult::Agent1);
        DOCTEST_CHECK(std::get<0>(result) == doctest::Approx(3000));
        DOCTEST_CHECK(std::get<1>(result) == doctest::Approx(-1000));
    }
}
}