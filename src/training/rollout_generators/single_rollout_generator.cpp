#include <memory>
#include <mutex>
#include <vector>

#include <cpprl/storage.h>
#include <cpprl/model/policy.h>
#include <doctest.h>

#include "single_rollout_generator.h"
#include "audio/audio_engine.h"
#include "environment/iecs_env.h"
#include "environment/serialization/serialize_body.h"
#include "graphics/colors.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "training/agents/iagent.h"
#include "training/agents/random_agent.h"

namespace ai
{
SingleRolloutGenerator::SingleRolloutGenerator(
    const IAgent &agent,
    std::unique_ptr<IEcsEnv> environment,
    const std::vector<std::unique_ptr<IAgent>> &opponent_pool,
    IAudioEngine &audio_engine,
    Random &rng,
    std::atomic<unsigned long long> *timestep)
    : agent(agent),
      audio_engine(audio_engine),
      environment(std::move(environment)),
      hidden_state(torch::zeros({agent.get_hidden_state_size(), 1})),
      last_observation(torch::zeros({agent.get_observation_size()})),
      opponent_pool(opponent_pool),
      reset_recently(false),
      rng(rng),
      score(0),
      should_stop(false),
      slow(false),
      start_position(false),
      timestep(timestep)
{
    std::lock_guard lock_guard(mutex);
    const auto opponent_index = rng.next_int(0, opponent_pool.size());
    opponent = opponent_pool[opponent_index].get();
    opponent_hidden_state = torch::zeros({opponent->get_hidden_state_size(), 1}),
    opponent_last_observation = torch::zeros({1, opponent->get_observation_size()});

    this->environment->set_body(0, agent.get_body_spec());

    auto opponent_json = opponent->get_body_spec();
    opponent_json["color_scheme"]["primary"] = {cl_red.r, cl_red.g, cl_red.b, cl_red.a};
    const auto transparent_red = set_alpha(cl_red, 0.2f);
    opponent_json["color_scheme"]["secondary"] = {transparent_red.r,
                                                  transparent_red.g,
                                                  transparent_red.b,
                                                  transparent_red.a};
    this->environment->set_body(1, opponent_json);

    this->environment->reset();
}

void SingleRolloutGenerator::fast_forward(unsigned int steps)
{
    auto observations = environment->reset().observations;
    last_observation = observations[0];
    opponent_last_observation = observations[1];

    for (unsigned int current_step = 0; current_step < steps; current_step++)
    {
        std::vector<torch::Tensor> actions;
        auto player_actions = torch::rand({1, agent.get_action_size()}).round();
        auto opponent_actions = torch::rand({1, opponent->get_action_size()}).round();
        actions = std::vector<torch::Tensor>{player_actions, opponent_actions};

        observations = environment->step(actions, 1.f / 10.f).observations;
        if (current_step == steps - 1)
        {
            last_observation = observations[0];
            opponent_last_observation = observations[1];
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
        if (should_stop)
        {
            should_stop = false;
            break;
        }
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

        EcsStepInfo step_info;
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
        opponent_last_observation = step_info.observations[opponent_index];
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
            auto opponent_json = opponent->get_body_spec();
            opponent_json["color_scheme"]["primary"] = {cl_red.r, cl_red.g, cl_red.b, cl_red.a};
            const auto transparent_red = set_alpha(cl_red, 0.2f);
            opponent_json["color_scheme"]["secondary"] = {transparent_red.r,
                                                          transparent_red.g,
                                                          transparent_red.b,
                                                          transparent_red.a};
            if (start_position)
            {
                environment->set_body(0, agent.get_body_spec());
                environment->set_body(1, opponent_json);
            }
            else
            {
                environment->set_body(1, agent.get_body_spec());
                environment->set_body(0, opponent_json);
            }
        }
        opponent_mask = 1 - dones;
        storage.insert(step_info.observations[player_index],
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
    environment->draw(renderer, audio_engine, !slow);
}

void SingleRolloutGenerator::stop()
{
    should_stop = true;
}

std::unique_ptr<ISingleRolloutGenerator> SingleRolloutGeneratorFactory::make(
    const IAgent &agent,
    std::unique_ptr<IEcsEnv> environment,
    const std::vector<std::unique_ptr<IAgent>> &opponent_pool,
    std::atomic<unsigned long long> *timestep)
{
    return std::make_unique<SingleRolloutGenerator>(agent,
                                                    std::move(environment),
                                                    opponent_pool,
                                                    audio_engine,
                                                    rng,
                                                    timestep);
}

using trompeloeil::_;

TEST_CASE("SingleRolloutGenerator")
{
    Random rng(0);
    MockAudioEngine audio_engine;
    RandomAgent agent(default_body(), rng, "Player");
    auto environment = std::make_unique<MockEcsEnv>();
    ALLOW_CALL(*environment, step(_, _))
        .RETURN(EcsStepInfo{torch::zeros({2, agent.get_body_spec()["num_observations"]}),
                            torch::zeros({2, 1}),
                            torch::zeros({2, 1}),
                            -1});
    ALLOW_CALL(*environment, forward(_));
    ALLOW_CALL(*environment, set_body(_, _));
    ALLOW_CALL(*environment, reset())
        .RETURN(EcsStepInfo{torch::zeros({2, agent.get_body_spec()["num_observations"]}),
                            torch::zeros({2, 1}),
                            torch::zeros({2, 1}),
                            -1});
    std::vector<std::unique_ptr<IAgent>> opponent_pool;
    opponent_pool.emplace_back(std::make_unique<RandomAgent>(default_body(), rng, "Opponent 1"));
    opponent_pool.emplace_back(std::make_unique<RandomAgent>(default_body(), rng, "Opponent 2"));
    SingleRolloutGenerator generator(agent,
                                     std::move(environment),
                                     opponent_pool,
                                     audio_engine,
                                     rng);

    SUBCASE("Generates the correct amount of frames")
    {
        const auto storage = generator.generate(7);

        const auto length = storage.get_actions().size(0);

        DOCTEST_CHECK(length == 7);
    }
}
}