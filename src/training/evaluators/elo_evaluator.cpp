#include <map>
#include <sstream>
#include <vector>

#include <doctest.h>
#include <cpprl/model/policy.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fmt/ostream.h>
#include <taskflow/taskflow.hpp>

#include "elo_evaluator.h"
#include "environment/serialization/serialize_body.h"
#include "misc/random.h"
#include "training/agents/iagent.h"
#include "training/agents/nn_agent.h"
#include "training/agents/random_agent.h"

namespace ai
{
const double elo_constant = 16;

double expected_win_chance(double a_rating, double b_rating)
{
    return 1. / (1. + pow(10., (b_rating - a_rating) / 400.));
}

std::tuple<double, double> calculate_elos(double a_rating,
                                          double b_rating,
                                          double k,
                                          EvaluationResult result)
{

    double a_win_chance = expected_win_chance(a_rating, b_rating);
    double b_win_chance = 1. - a_win_chance;

    double new_a_rating;
    double new_b_rating;
    if (result == EvaluationResult::Agent1)
    {
        new_a_rating = a_rating + k * (1 - a_win_chance);
        new_b_rating = b_rating + k * (0 - b_win_chance);
    }
    else if (result == EvaluationResult::Agent2)
    {
        new_a_rating = a_rating + k * (0 - a_win_chance);
        new_b_rating = b_rating + k * (1 - b_win_chance);
    }
    else
    {
        new_a_rating = a_rating + k * (0.5 - a_win_chance);
        new_b_rating = b_rating + k * (0.5 - b_win_chance);
    }

    return {new_a_rating, new_b_rating};
}

EloEvaluator::EloEvaluator(Random &rng) : rng(rng) {}

double EloEvaluator::evaluate(IAgent &agent,
                              const std::vector<IAgent *> &new_opponents,
                              unsigned int number_of_trials)
{
    if (main_agent == nullptr)
    {
        // Initialize main agent
        main_agent = agent.clone();
        elos[main_agent.get()] = 0;
    }
    else
    {
        // Update main agent
        auto new_main_agent_temp = agent.clone();
        elos[new_main_agent_temp.get()] = elos[main_agent.get()];
        elos.erase(main_agent.get());
        main_agent = std::move(new_main_agent_temp);
    }

    // Add new opponents to opponent pool
    for (const auto &opponent : new_opponents)
    {
        opponents.push_back(opponent->clone());
        elos[opponents.back().get()] = 0;
    }

    struct Evaluation
    {
        IAgent *agent_1;
        IAgent *agent_2;
        EvaluationResult result = EvaluationResult::Draw;
    };
    std::vector<Evaluation> evaluations;

    // Select players
    for (unsigned int i = 0; i < number_of_trials; ++i)
    {
        // Select randomly from opponent pool
        // TODO: Make it impossible for an agent to play against itself
        IAgent *agent_1 = opponents[rng.next_int(0, opponents.size())].get();
        IAgent *agent_2 = opponents[rng.next_int(0, opponents.size())].get();

        // 1 in 10 chance to play against the main agent
        if (rng.next_float(0, 1) < 0.1)
        {
            if (rng.next_float(0, 1) < 0.5)
            {
                agent_1 = main_agent.get();
            }
            else
            {
                agent_2 = main_agent.get();
            }
        }

        evaluations.push_back({agent_1, agent_2});
    }

    // Create evaluation tasks
    tf::Executor executor;
    tf::Taskflow task_flow;

    for (auto &evaluation : evaluations)
    {
        task_flow.emplace([&] {
            auto result = Evaluator::evaluate(*evaluation.agent_1, *evaluation.agent_2);
            evaluation.result = result;
        });
    }

    // Run evaluation tasks
    executor.run(task_flow);
    executor.wait_for_all();

    // Calculate Elos
    for (const auto &evaluation : evaluations)
    {
        std::tie(elos[evaluation.agent_1],
                 elos[evaluation.agent_2]) = calculate_elos(elos[evaluation.agent_1],
                                                            elos[evaluation.agent_2],
                                                            elo_constant,
                                                            evaluation.result);
    }

    spdlog::debug("{}: {}", main_agent->get_name(), elos[main_agent.get()]);
    for (const auto &opponent : opponents)
    {
        spdlog::debug("{}: {}", opponent->get_name(), elos[opponent.get()]);
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

    SUBCASE("Calculates elos within expected boundaries when evaluating random agents")
    {
        Random rng(0);
        EloEvaluator evaluator(rng);

        RandomAgent agent_1(default_body(), rng, "Agent 1");
        RandomAgent agent_2(default_body(), rng, "Agent 2");
        RandomAgent agent_3(default_body(), rng, "Agent 3");
        RandomAgent agent_4(default_body(), rng, "Agent 4");

        std::vector<IAgent *> new_opponents;

        new_opponents.push_back(&agent_2);
        evaluator.evaluate(agent_1, new_opponents, 16);
        new_opponents[0] = &agent_3;
        evaluator.evaluate(agent_1, new_opponents, 16);
        new_opponents[0] = &agent_4;
        auto elo = evaluator.evaluate(agent_1, new_opponents, 16);

        DOCTEST_CHECK(elo < 40);
        DOCTEST_CHECK(elo > -40);
    }
}
}