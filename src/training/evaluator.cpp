#include <doctest.h>
#include <nlohmann/json.hpp>

#include "evaluator.h"
#include "misc/random.h"
#include "training/agents/iagent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/test_body.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"

namespace SingularityTrainer
{
Evaluator::Evaluator(IEnvironmentFactory &env_factory)
    : env_factory(env_factory) {}

EvaluationResult Evaluator::evaluate(IAgent &agent_1,
                                     IAgent &agent_2,
                                     const nlohmann::json &body_1_spec,
                                     const nlohmann::json &body_2_spec,
                                     int number_of_trials)
{
    return {0, 0, 0};
}

TEST_CASE("Evaluator")
{
    SUBCASE("evaluate() runs the correct number of trials")
    {
        KothEnvFactory env_factory(100);
        Evaluator evaluator(env_factory);

        Random rng(0);
        TestBody body(rng);
        auto body_spec = body.to_json();

        RandomAgent agent(body.get_actions().size(), rng);

        auto results = evaluator.evaluate(agent,
                                          agent,
                                          body_spec,
                                          body_spec,
                                          10);

        int number_of_trials = results.agent_1 + results.agent_2 + results.draw;
        DOCTEST_CHECK(number_of_trials == 10);
    }
}
}