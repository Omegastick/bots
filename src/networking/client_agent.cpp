#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>
#include <torch/torch.h>

#include "client_agent.h"
#include "misc/random.h"
#include "training/agents/iagent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"

namespace SingularityTrainer
{
ClientAgent::ClientAgent(std::unique_ptr<IAgent> agent,
                         int agent_number,
                         std::unique_ptr<IEnvironment> env)
    : agent(std::move(agent)),
      agent_number(agent_number),
      env(std::move(env)),
      hidden_state(torch::zeros({this->agent->get_hidden_state_size()})) {}

std::vector<int> ClientAgent::get_action(const EnvState &env_state)
{
    env->set_state(env_state);
    auto observation = env->get_bodies()[agent_number]->get_observation();

    auto observation_tensor = torch::from_blob(observation.data(),
                                               {static_cast<long>(observation.size())},
                                               torch::kFloat);
    auto act_result = agent->act(observation_tensor,
                                 hidden_state,
                                 torch::ones({1}));
    hidden_state = std::get<1>(act_result);
    auto actions_tensor = std::get<0>(act_result).to(torch::kInt);
    return std::vector<int>(actions_tensor.data<int>(),
                            actions_tensor.data<int>() + actions_tensor.numel());
}

void ClientAgent::set_bodies(const std::vector<nlohmann::json> &body_specs)
{
    for (int i = 0; i < body_specs.size(); ++i)
    {
        env->get_bodies()[i]->load_json(body_specs[i]);
    }
}

TEST_CASE("ClientAgent")
{
    Random rng(0);
    TestBodyFactory body_factory(rng);
    KothEnvFactory env_factory(10, body_factory);
    auto env = env_factory.make();
    auto agent = std::make_unique<RandomAgent>(env->get_bodies()[0]->to_json(), rng, "Random agent");

    ClientAgent client_agent(std::move(agent), 1, std::move(env));

    SUBCASE("get_action() returns correctly sized action")
    {
        EnvState env_state(std::vector<b2Transform>{b2Transform(b2Vec2(-5, -5), b2Rot(0)),
                                                    b2Transform(b2Vec2(5, 5), b2Rot(1))},
                           std::unordered_map<unsigned int, b2Transform>{},
                           0);
        auto action = client_agent.get_action(env_state);

        DOCTEST_CHECK(action.size() == 4);
    }
}
}