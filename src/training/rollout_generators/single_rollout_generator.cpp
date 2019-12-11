#include <memory>
#include <mutex>
#include <vector>

#include <cpprl/storage.h>
#include <cpprl/model/policy.h>
#include <doctest.h>

#include "single_rollout_generator.h"
#include "training/agents/iagent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/test_body.h"
#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
SingleRolloutGenerator::SingleRolloutGenerator(
    const IAgent &agent,
    std::unique_ptr<IEnvironment> environment,
    const std::vector<std::unique_ptr<IAgent>> &opponent_pool,
    Random &rng,
    bool skip_start,
    std::atomic<unsigned long long> *timestep)
    : agent(agent),
      environment(std::move(environment)),
      hidden_state(torch::zeros({agent.get_hidden_state_size(), 1})),
      last_observation(torch::zeros({agent.get_observation_size()})),
      opponent_pool(opponent_pool),
      reset_recently(false),
      rng(rng),
      score(0),
      slow(false),
      start_position(false),
      timestep(timestep)
{
    std::lock_guard lock_guard(mutex);
    const auto opponent_index = rng.next_int(0, opponent_pool.size());
    opponent = opponent_pool[opponent_index].get();
    opponent_hidden_state = torch::zeros({opponent->get_hidden_state_size(), 1}),
    opponent_last_observation = torch::zeros({1, opponent->get_observation_size()});

    auto bodies = this->environment->get_bodies();
    bodies[0]->load_json(agent.get_body_spec());
    bodies[1]->load_json(opponent->get_body_spec());
    bodies[1]->set_color({cl_red, set_alpha(cl_red, 0.2f)});

    auto env_observation = this->environment->reset().observation;
    last_observation = env_observation[0];
    opponent_last_observation = env_observation[1];

    if (!skip_start)
    {
        return;
    }
    // Run the environment for a bit
    const int pre_steps = rng.next_int(1, 600);
    for (int current_step = 0; current_step < pre_steps; current_step++)
    {
        std::vector<torch::Tensor> actions;
        auto player_actions = torch::rand({1, agent.get_action_size()}).round();
        auto opponent_actions = torch::rand({1, opponent->get_action_size()}).round();
        actions = std::vector<torch::Tensor>{player_actions, opponent_actions};

        env_observation = this->environment->step(actions, 1.f / 10.f).observation;
        if (current_step == pre_steps - 1)
        {
            last_observation = env_observation[0];
            opponent_last_observation = env_observation[1];
        }
    }
}

cpprl::RolloutStorage SingleRolloutGenerator::generate(unsigned long length)
{
    cpprl::RolloutStorage storage(
        length,
        1,
        c10::IntArrayRef{agent.get_body_spec()["num_observations"]},
        cpprl::ActionSpace{"MultiBinary", {agent.get_body_spec()["num_actions"]}},
        64,
        torch::kCPU);
    storage.set_first_observation(last_observation);

    for (unsigned long step = 0; step < length; ++step)
    {
        // Get action from agent
        ActResult act_result, opponent_act_result;
        {
            torch::NoGradGuard no_grad;
            act_result = agent.act(storage.get_observations()[step],
                                   storage.get_hidden_states()[step],
                                   storage.get_masks()[step]);
            opponent_act_result = opponent->act(opponent_last_observation,
                                                opponent_hidden_state,
                                                opponent_mask);
        }

        // Get opponent action
        opponent_hidden_state = opponent_act_result.hidden_state;

        // Step environment
        torch::Tensor dones = torch::zeros({1, 1});
        torch::Tensor opponent_dones = torch::zeros({1, 1});
        torch::Tensor rewards = torch::zeros({1, 1});

        StepInfo step_info;
        {
            std::lock_guard lock_guard(mutex);
            auto &player_actions = act_result.action;
            auto &opponent_actions = opponent_act_result.action;
            std::vector<torch::Tensor> actions;
            if (start_position)
            {
                actions = {player_actions, opponent_actions};
            }
            else
            {
                actions = {opponent_actions, player_actions};
            }
            step_info = environment->step(actions, 1.f / 60.f);
        }
        if (slow)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
        }
        for (int mini_step = 0; mini_step < 5; ++mini_step)
        {
            {
                std::lock_guard lock_guard(mutex);
                environment->forward(1.f / 60.f);
            }
            if (slow)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
            }
        }
        int player_index = start_position ? 0 : 1;
        int opponent_index = start_position ? 1 : 0;
        dones = step_info.done[player_index];
        rewards = step_info.reward[player_index];
        opponent_last_observation = step_info.observation[opponent_index];
        opponent_dones = step_info.done[opponent_index];
        score = score + step_info.reward[player_index].item().toFloat();
        if (step_info.done[player_index].item().toBool())
        {
            std::lock_guard lock_guard(mutex);
            reset_recently = true;
            score = 0;
            environment->reset();
            int selected_opponent = rng.next_int(0, opponent_pool.size());
            opponent = opponent_pool[selected_opponent].get();
            opponent_hidden_state = torch::zeros({opponent->get_hidden_state_size(), 1});
            start_position = rng.next_bool(0.5);
            auto bodies = environment->get_bodies();
            if (start_position)
            {
                bodies[0]->load_json(agent.get_body_spec());
                bodies[1]->load_json(opponent->get_body_spec());
                bodies[1]->set_color({cl_red, set_alpha(cl_red, 0.2f)});
            }
            else
            {
                bodies[0]->load_json(opponent->get_body_spec());
                bodies[0]->set_color({cl_red, set_alpha(cl_red, 0.2f)});
                bodies[1]->load_json(agent.get_body_spec());
            }
        }
        opponent_mask = 1 - dones;
        storage.insert(step_info.observation[player_index],
                       act_result.hidden_state,
                       act_result.action,
                       act_result.log_probs,
                       act_result.value,
                       rewards,
                       1 - dones);
        if (timestep)
        {
            (*timestep)++;
        }
    }

    return storage;
}

void SingleRolloutGenerator::draw(Renderer &renderer, bool /*lightweight*/)
{
    std::lock_guard lock_guard(mutex);
    environment->draw(renderer, !slow);
}

using trompeloeil::_;

TEST_CASE("SingleRolloutGenerator")
{
    Random rng(0);
    TestBody body(rng);
    const auto body_json = body.to_json();
    RandomAgent agent(body_json, rng, "Player");
    auto environment = std::make_unique<MockEnvironment>();
    ALLOW_CALL(*environment, step(_, _))
        .RETURN(StepInfo{{},
                         {torch::zeros({1, 23}), torch::zeros({1, 23})},
                         torch::zeros({2, 1}),
                         torch::zeros({2, 1}),
                         -1});
    ALLOW_CALL(*environment, forward(_));
    ALLOW_CALL(*environment, get_bodies())
        .LR_RETURN(std::vector<Body *>{&body, &body});
    ALLOW_CALL(*environment, reset())
        .RETURN(StepInfo{{},
                         {torch::zeros({1, 23}), torch::zeros({1, 23})},
                         torch::zeros({2, 1}),
                         torch::zeros({2, 1}),
                         -1});
    std::vector<std::unique_ptr<IAgent>> opponent_pool;
    opponent_pool.emplace_back(std::make_unique<RandomAgent>(body_json, rng, "Opponent 1"));
    opponent_pool.emplace_back(std::make_unique<RandomAgent>(body_json, rng, "Opponent 2"));
    SingleRolloutGenerator generator(agent, std::move(environment), opponent_pool, rng, true);

    SUBCASE("Generates the correct amount of frames")
    {
        const auto storage = generator.generate(7);

        const auto length = storage.get_actions().size(0);

        DOCTEST_CHECK(length == 7);
    }
}
}