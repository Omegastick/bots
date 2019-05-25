#include <memory>
#include <chrono>

#include <cpprl/cpprl.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>
#include <torch/torch.h>

#include "misc/random.h"
#include "misc/resource_manager.h"
#include "training/environments/koth_env.h"
#include "training/trainers/quick_trainer.h"
#include "training/score_processor.h"
#include "misc/utilities.h"
#include "misc/date.h"

namespace SingularityTrainer
{
static const int batch_size = 1024;
static const float entropy_coef = 0.001;
static const int epochs = 10;
static const int game_length = 600;
static const bool recurrent = false;

QuickTrainer::QuickTrainer(Random *rng, int env_count)
    : agents_per_env(2),
      policy(nullptr),
      rollout_storage(batch_size, env_count * agents_per_env, {23}, {"MultiBinary", {4}}, {24}, torch::kCPU),
      waiting(false),
      env_count(env_count),
      frame_counter(0),
      action_frame_counter(0),
      rng(rng),
      elapsed_time(0),
      score_processor(std::make_unique<ScoreProcessor>(1, 0.99)),
      last_update_time(std::chrono::high_resolution_clock::now())
{
    torch::manual_seed(0);

    for (int i = 0; i < env_count; ++i)
    {
        environments.push_back(std::make_unique<KothEnv>(460, 40, 1, game_length, i));
    }
    env_scores.resize(env_count);
}

QuickTrainer::~QuickTrainer() {}

void QuickTrainer::begin_training()
{
    std::vector<std::future<StepInfo>> observation_futures(env_count);
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        environments[i]->start_thread();
        observation_futures[i] = environments[i]->reset();

        // Start each environment with a different number of random steps to decorrelate the environments
        for (unsigned int current_step = 0; current_step < i * 12; current_step++)
        {
            auto actions = torch::rand({agents_per_env, 4});
            observation_futures[i] = environments[i]->step(actions, 1.f / 10.f);
        }
    }
    observations = torch::zeros({env_count * agents_per_env, 23});
    for (int i = 0; i < env_count; ++i)
    {
        auto env_observation = observation_futures[i].get();
        for (int j = 0; j < agents_per_env; ++j)
        {
            observations[i * agents_per_env + j] = env_observation.observation[j];
        }
    }

    nn_base = std::make_shared<cpprl::MlpBase>(23, recurrent, 24);
    policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {4}}, nn_base);
    algorithm = std::make_unique<cpprl::PPO>(policy, 0.2, epochs, env_count, 0.5, entropy_coef, 3e-4);

    rollout_storage.set_first_observation(observations);
}

void QuickTrainer::end_training() {}

void QuickTrainer::step()
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
    } while (std::chrono::high_resolution_clock::now() - clock_start < std::chrono::duration<double>(1. / 60.));
}

void QuickTrainer::slow_step()
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

void QuickTrainer::save_model()
{
    auto date_time_string = date::format("%F-%H-%M", std::chrono::system_clock::now());
    torch::save(policy, date_time_string + ".pth");
}

void QuickTrainer::action_update()
{
    // Get actions from policy
    std::vector<torch::Tensor> act_result;
    {
        torch::NoGradGuard no_grad;
        act_result = policy->act(rollout_storage.get_observations()[action_frame_counter % batch_size],
                                 rollout_storage.get_hidden_states()[action_frame_counter % batch_size],
                                 rollout_storage.get_masks()[action_frame_counter % batch_size]);
    }

    // Step environments
    torch::Tensor dones = torch::zeros({env_count * agents_per_env, 1});
    torch::Tensor rewards = torch::zeros({env_count * agents_per_env, 1});
    std::vector<std::future<StepInfo>> step_info_futures(environments.size());
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        step_info_futures[i] = environments[i]->step(act_result[1].narrow(0, i, agents_per_env).view({agents_per_env, -1}),
                                                     1. / 60.);
    }
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        auto step_info = step_info_futures[i].get();
        for (int j = 0; j < agents_per_env; ++j)
        {
            observations[i * agents_per_env + j] = step_info.observation[j];
            dones[i * agents_per_env + j] = step_info.done[j];
            rewards[i * agents_per_env + j] = step_info.reward[j];
        }
        for (unsigned int j = 0; j < step_info.reward.size(0); ++j)
        {
            env_scores[i] += step_info.reward[j].item().toFloat();
            if (step_info.done[j].item().toBool())
            {
                score_processor->add_score(0, env_scores[i]);
                env_scores[i] = 0;
            }
        }
    }

    rollout_storage.insert(observations,
                           act_result[3],
                           act_result[1],
                           act_result[2],
                           act_result[0],
                           rewards,
                           1 - dones);

    action_frame_counter++;

    // Train
    if (action_frame_counter % batch_size == 0)
    {
        auto update_start_time = std::chrono::high_resolution_clock::now();

        torch::Tensor next_value;
        {
            torch::NoGradGuard no_grad;
            next_value = policy->get_values(
                                   rollout_storage.get_observations()[-1],
                                   rollout_storage.get_hidden_states()[-1],
                                   rollout_storage.get_masks()[-1])
                             .detach();
        }
        rollout_storage.compute_returns(next_value, true, 0.99, 0.95);

        auto update_data = algorithm->update(rollout_storage);
        rollout_storage.after_update();

        spdlog::info("---");
        spdlog::info("Total frames: {}", action_frame_counter * env_count);
        double fps = (batch_size * env_count) / static_cast<std::chrono::duration<double>>(update_start_time - last_update_time).count();
        spdlog::info("FPS: {:.2f}", fps);
        for (const auto &datum : update_data)
        {
            spdlog::info("{}: {}", datum.name, datum.value);
        }
        std::vector<float> score_averages = score_processor->get_scores();
        spdlog::info("Average reward: {:.2f}", fmt::join(score_averages, " "));
        auto update_end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> update_duration = update_end_time - update_start_time;
        spdlog::info("Update took {:.2f}s", update_duration.count());

        last_update_time = update_end_time;
    }
}
}