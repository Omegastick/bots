#include <chrono>
#include <filesystem>
#include <memory>

#include <Box2D/Box2D.h>
#include <cpprl/cpprl.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>
#include <torch/torch.h>

#include "trainer.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "training/agents/iagent.h"
#include "training/agents/nn_agent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"
#include "training/checkpointer.h"
#include "training/environments/ienvironment.h"
#include "training/evaluators/basic_evaluator.h"
#include "training/score_processor.h"
#include "training/training_program.h"
#include "misc/date.h"
#include "misc/random.h"
#include "misc/utilities.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
const int body_per_env = 2;
const int game_length = 600;
const bool recurrent = false;

Trainer::Trainer(TrainingProgram program,
                 BodyFactory &body_factory,
                 Checkpointer &checkpointer,
                 IEnvironmentFactory &env_factory,
                 BasicEvaluator &evaluator,
                 Random &rng)
    : action_frame_counter(0),
      checkpointer(checkpointer),
      elapsed_time(0),
      env_count(program.hyper_parameters.num_env),
      evaluator(evaluator),
      frame_counter(0),
      last_save_time(std::chrono::high_resolution_clock::now()),
      last_update_time(std::chrono::high_resolution_clock::now()),
      policy(nullptr),
      previous_checkpoint(program.checkpoint),
      program(program),
      rng(rng),
      waiting(false)
{
    torch::manual_seed(0);

    // Initialize rollout storage
    b2World b2_world({0, 0});
    auto temp_body = body_factory.make(b2_world, rng);
    temp_body->load_json(program.body);
    int num_observations = temp_body->get_observation().size();
    int num_actions = temp_body->get_input_count();
    rollout_storage = std::make_unique<cpprl::RolloutStorage>(program.hyper_parameters.batch_size,
                                                              program.hyper_parameters.num_env,
                                                              c10::IntArrayRef{num_observations},
                                                              cpprl::ActionSpace{"MultiBinary", {num_actions}},
                                                              64,
                                                              torch::kCPU);

    // Initialize environments
    for (int i = 0; i < env_count; ++i)
    {
        auto world = std::make_unique<b2World>(b2Vec2_zero);
        auto rng = std::make_unique<Random>(i);
        std::vector<std::unique_ptr<Body>> bodies;
        bodies.push_back(body_factory.make(*world, *rng));
        bodies.push_back(body_factory.make(*world, *rng));
        bodies[0]->load_json(program.body);
        bodies[1]->load_json(program.body);
        environments.push_back(env_factory.make(std::move(rng),
                                                std::move(world),
                                                std::move(bodies),
                                                program.reward_config));
    }
    env_scores.resize(env_count);

    // Initialize opponent pool
    opponent_pool.push_back(std::make_unique<RandomAgent>(program.body, rng));
    for (const auto &checkpoint_path : program.opponent_pool)
    {
        auto policy = checkpointer.load(checkpoint_path).policy;
        opponent_pool.push_back(std::make_unique<NNAgent>(policy, program.body));
    }

    // Initialize opponents
    opponents.resize(env_count);
    for (int i = 0; i < env_count; ++i)
    {
        int selected_opponent = rng.next_int(0, opponent_pool.size());
        opponents[i] = opponent_pool[selected_opponent].get();
        opponent_hidden_states.push_back(torch::zeros({opponents[i]->get_hidden_state_size(), 1}));
    }
    opponent_masks = torch::ones({env_count, 1});

    observations = torch::zeros({env_count, num_observations});
    opponent_observations.clear();
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        auto env_observation = environments[i]->reset().observation;
        observations[i] = env_observation[0];
        opponent_observations.push_back(env_observation[1]);

        // Start each environment with a different number of random steps to decorrelate the environments
        for (unsigned int current_step = 0; current_step < i * 12; current_step++)
        {
            std::vector<torch::Tensor> actions{
                torch::rand({1, num_actions}),
                torch::rand({1, opponents[i]->get_action_size()})};
            env_observation = environments[i]->step(actions, 1.f / 10.f).observation;
            if (current_step == (i * 12) - 1)
            {
                observations[i] = env_observation[0];
                opponent_observations[i] = env_observation[1];
            }
        }
    }

    if (!program.checkpoint.empty())
    {
        spdlog::debug("Loading {}", program.checkpoint);
        auto checkpoint = checkpointer.load(program.checkpoint);
        policy = checkpoint.policy;
    }
    else
    {
        spdlog::debug("Making new agent");
        auto nn_base = std::make_shared<cpprl::MlpBase>(num_observations, recurrent);
        policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {num_actions}}, nn_base);
    }

    if (program.hyper_parameters.algorithm == Algorithm::A2C)
    {
        algorithm = std::make_unique<cpprl::A2C>(policy,
                                                 program.hyper_parameters.actor_loss_coef,
                                                 program.hyper_parameters.value_loss_coef,
                                                 program.hyper_parameters.entropy_coef,
                                                 program.hyper_parameters.learning_rate);
    }
    else if (program.hyper_parameters.algorithm == Algorithm::PPO)
    {
        algorithm = std::make_unique<cpprl::PPO>(policy,
                                                 program.hyper_parameters.clip_param,
                                                 program.hyper_parameters.num_epoch,
                                                 program.hyper_parameters.num_minibatch,
                                                 program.hyper_parameters.actor_loss_coef,
                                                 program.hyper_parameters.value_loss_coef,
                                                 program.hyper_parameters.entropy_coef,
                                                 program.hyper_parameters.learning_rate);
    }
    else
    {
        throw std::runtime_error("Algorithm not supported");
    }

    rollout_storage->set_first_observation(observations);
}

float Trainer::evaluate(int number_of_trials)
{
    auto results = evaluator.evaluate(policy, program.body, number_of_trials);
    return (results.agent_1 + (results.draw * 0.5)) / (results.agent_1 + results.agent_2 + results.draw);
}

std::vector<float> Trainer::get_observation()
{
    auto observation = rollout_storage->get_observations()[action_frame_counter % program.hyper_parameters.batch_size][0];
    return std::vector<float>(observation.data<float>(), observation.data<float>() + observation.numel());
}

void Trainer::step()
{
    if (waiting)
    {
        return;
    }
    auto clock_start = std::chrono::high_resolution_clock::now();
    do
    {
        action_update();
        for (int i = 0; i < 5; ++i)
        {
            for (const auto &environment : environments)
            {
                environment->forward(1. / 60.);
            }
        }
        if (waiting)
        {
            break;
        }
    } while (std::chrono::high_resolution_clock::now() - clock_start < std::chrono::milliseconds(1000 / 60));
}

void Trainer::slow_step()
{
    if (waiting)
    {
        return;
    }
    if (++frame_counter >= 6)
    {
        frame_counter = 0;
        action_update();
    }
    else
    {
        for (const auto &environment : environments)
        {
            environment->forward(1. / 60.);
        }
    }
}

std::filesystem::path Trainer::save_model(std::filesystem::path directory)
{
    previous_checkpoint = checkpointer.save(policy, program.body, {}, previous_checkpoint, directory);

    return previous_checkpoint;
}

void Trainer::action_update()
{
    // Get actions from policy
    std::vector<torch::Tensor> act_result;
    {
        torch::NoGradGuard no_grad;
        act_result = policy->act(rollout_storage->get_observations()[action_frame_counter % program.hyper_parameters.batch_size],
                                 rollout_storage->get_hidden_states()[action_frame_counter % program.hyper_parameters.batch_size],
                                 rollout_storage->get_masks()[action_frame_counter % program.hyper_parameters.batch_size]);
    }
    std::vector<torch::Tensor> opponent_actions;
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        auto opponent_act_result = opponents[i]->act(opponent_observations[i],
                                                     opponent_hidden_states[i],
                                                     opponent_masks[i]);
        opponent_actions.push_back(std::get<0>(opponent_act_result));
        opponent_hidden_states[i] = std::get<1>(opponent_act_result);
    }

    // Step environments
    torch::Tensor dones = torch::zeros({env_count, 1});
    torch::Tensor opponent_dones = torch::zeros({env_count, 1});
    torch::Tensor rewards = torch::zeros({env_count, 1});
    std::vector<StepInfo> step_infos(environments.size());
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        step_infos[i] = environments[i]->step({act_result[1][i], opponent_actions[i]},
                                              1. / 60.);
    }
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        auto &step_info = step_infos[i];
        observations[i] = step_info.observation[0];
        dones[i] = step_info.done[0];
        rewards[i] = step_info.reward[0];
        opponent_observations[i] = step_info.observation[1];
        opponent_dones[i] = step_info.done[1];
        env_scores[i] += step_info.reward[0].item().toFloat();
        if (step_info.done[0].item().toBool())
        {
            env_scores[i] = 0;
            environments[i]->reset();
            int selected_opponent = rng.next_int(0, opponent_pool.size());
            opponents[i] = opponent_pool[selected_opponent].get();
        }
    }

    rollout_storage->insert(observations,
                            act_result[3],
                            act_result[1],
                            act_result[2],
                            act_result[0],
                            rewards,
                            1 - dones);

    action_frame_counter++;

    // Train
    if (action_frame_counter % program.hyper_parameters.batch_size == 0)
    {
        auto update_start_time = std::chrono::high_resolution_clock::now();

        torch::Tensor next_value;
        {
            torch::NoGradGuard no_grad;
            next_value = policy->get_values(
                                   rollout_storage->get_observations()[-1],
                                   rollout_storage->get_hidden_states()[-1],
                                   rollout_storage->get_masks()[-1])
                             .detach();
        }
        rollout_storage->compute_returns(next_value, true, program.hyper_parameters.discount_factor, 0.95);

        auto update_data = algorithm->update(*rollout_storage);
        rollout_storage->after_update();

        spdlog::info("---");
        spdlog::info("Total frames: {}", action_frame_counter * env_count);
        double fps = (program.hyper_parameters.batch_size * env_count) / static_cast<std::chrono::duration<double>>(update_start_time - last_update_time).count();
        spdlog::info("FPS: {:.2f}", fps);
        for (const auto &datum : update_data)
        {
            spdlog::info("{}: {}", datum.name, datum.value);
        }
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> update_duration = now - update_start_time;
        spdlog::info("Update took {:.2f}s", update_duration.count());

        last_update_time = now;

        if (now - last_save_time > std::chrono::minutes(program.minutes_per_checkpoint))
        {
            auto checkpoint_path = save_model();
            opponent_pool.push_back(std::make_unique<NNAgent>(policy, program.body));
        }
    }
}
}