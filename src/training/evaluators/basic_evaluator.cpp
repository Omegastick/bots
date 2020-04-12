#include <mutex>

#include <doctest.h>
#include <cpprl/model/policy.h>
#include <nlohmann/json.hpp>
#include <taskflow/taskflow.hpp>

#include "basic_evaluator.h"
#include "environment/serialization/serialize_body.h"
#include "misc/random.h"
#include "training/agents/nn_agent.h"
#include "training/agents/random_agent.h"

namespace ai
{
BasicEvaluator::BasicEvaluator(Random &rng) : rng(rng) {}

double BasicEvaluator::evaluate(const IAgent &agent, int number_of_trials)
{
    RandomAgent random_agent(default_body(), rng, "Random Agent");

    std::vector<EvaluationResult> results;
    std::mutex results_mutex;

    tf::Executor executor;
    tf::Taskflow task_flow;
    for (int i = 0; i < number_of_trials; ++i)
    {
        task_flow.emplace([&] {
            auto result = Evaluator::evaluate(agent, random_agent);
            {
                std::lock_guard lock_guard(results_mutex);
                results.push_back(result);
            }
        });
    }

    executor.run(task_flow);
    executor.wait_for_all();

    double total = 0;
    for (const auto &result : results)
    {
        if (result == EvaluationResult::Agent1)
        {
            return total + 1;
        }
        else if (result == EvaluationResult::Draw)
        {
            return total + 0.5;
        }

        return total;
    }

    return total / results.size();
}

TEST_CASE("BasicEvaluator")
{
    SUBCASE("evaluate() runs the correct number of trials")
    {
        Random rng(0);
        BasicEvaluator evaluator(rng);

        RandomAgent agent(default_body(), rng, "Agent");

        auto result = evaluator.evaluate(agent, 10);

        DOCTEST_CHECK(result >= 0);
        DOCTEST_CHECK(result <= 10);
    }
}
}