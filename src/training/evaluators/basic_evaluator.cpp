#include <mutex>

#include <doctest.h>
#include <cpprl/model/policy.h>
#include <nlohmann/json.hpp>
#include <taskflow/taskflow.hpp>

#include "basic_evaluator.h"
#include "audio/audio_engine.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "training/agents/nn_agent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"
#include "training/environments/koth_env.h"

namespace ai
{
BasicEvaluator::BasicEvaluator(BodyFactory &body_factory,
                               IEnvironmentFactory &env_factory,
                               Random &rng)
    : Evaluator(body_factory, env_factory),
      rng(rng) {}

double BasicEvaluator::evaluate(const IAgent &agent,
                                int number_of_trials)
{
    MockAudioEngine audio_engine;
    ALLOW_CALL(audio_engine, play(ANY(std::string)))
        .LR_RETURN(SoundHandle(audio_engine, 0));
    MockBulletFactory bullet_factory;
    ModuleFactory module_factory(audio_engine, bullet_factory, rng);
    TestBody test_body(module_factory, rng);
    auto test_body_json = test_body.to_json();
    RandomAgent random_agent(test_body_json, rng, "Random Agent");

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
        MockAudioEngine audio_engine;
        BulletFactory bullet_factory(audio_engine);
        ModuleFactory module_factory(audio_engine, bullet_factory, rng);
        BodyFactory body_factory(module_factory, rng);
        KothEnvFactory env_factory(10, audio_engine, body_factory, bullet_factory);
        BasicEvaluator evaluator(body_factory, env_factory, rng);

        TestBody body(module_factory, rng);
        auto body_spec = body.to_json();
        RandomAgent agent(body_spec, rng, "Agent");

        auto result = evaluator.evaluate(agent, 10);

        DOCTEST_CHECK(result >= 0);
        DOCTEST_CHECK(result <= 10);
    }
}
}