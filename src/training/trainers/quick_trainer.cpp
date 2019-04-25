#include <memory>
#include <chrono>

#include <cpprl/cpprl.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>
#include <torch/torch.h>

#include "random.h"
#include "resource_manager.h"
#include "training/environments/target_env.h"
#include "training/trainers/quick_trainer.h"
#include "training/score_processor.h"
#include "utilities.h"
#include "date.h"

namespace SingularityTrainer
{
QuickTrainer::QuickTrainer(Random *rng, int env_count)
    : policy(nullptr),
      rollout_storage(512, env_count, {23}, {"MultiBinary", {4}}, {24}, torch::kCPU),
      waiting(false),
      env_count(env_count),
      frame_counter(0),
      action_frame_counter(0),
      rng(rng),
      elapsed_time(0),
      score_processor(std::make_unique<ScoreProcessor>(1, 0.99)),
      agents_per_env(1)
{
    for (int i = 0; i < env_count; ++i)
    {
        environments.push_back(std::make_unique<TargetEnv>(460, 40, 1, 100, i));
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
    std::vector<torch::Tensor> observations_(env_count);
    for (int i = 0; i < env_count; ++i)
    {
        observations_[i] = observation_futures[i].get().observation;
    }
    observations = torch::stack(observations_);

    nn_base = std::make_shared<cpprl::MlpBase>(23, false, 24);
    policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {4}}, nn_base);
    algorithm = std::make_unique<cpprl::PPO>(policy, 0.2, 10, env_count, 0.5, 0.01, 1e-4);

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
        act_result = policy->act(rollout_storage.get_observations()[action_frame_counter % 512],
                                 rollout_storage.get_hidden_states()[action_frame_counter % 512],
                                 rollout_storage.get_masks()[action_frame_counter % 512]);
    }

    // Step environments
    torch::Tensor dones = torch::zeros({env_count * agents_per_env, 1});
    torch::Tensor rewards = torch::zeros({env_count * agents_per_env, 1});
    std::vector<std::future<StepInfo>> step_info_futures(environments.size());
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        step_info_futures[i] = environments[i]->step(act_result[1][i].view({agents_per_env, -1}),
                                                     1. / 60.);
    }
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        auto step_info = step_info_futures[i].get();
        observations[i] = step_info.observation;
        for (int j = 0; j < agents_per_env; ++j)
        {
            dones[i * agents_per_env + j][0] = step_info.done[j];
            rewards[i * agents_per_env + j][0] = step_info.reward[j];
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
    if (action_frame_counter % 512 == 0)
    {
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
        for (const auto &datum : update_data)
        {
            spdlog::info("{}: {}", datum.name, datum.value);
        }
        std::vector<float> score_averages = score_processor->get_scores();
        spdlog::info("Average reward: {}", fmt::join(score_averages, " "));
    }

    // // Get actions from training server
    // std::shared_ptr<GetActionsParam> get_actions_param = std::make_shared<GetActionsParam>();
    // get_actions_param->inputs = interleave_vectors<std::vector<float>>(observations);
    // get_actions_param->session_id = 0;
    // Request<GetActionsParam> get_actions_request("get_actions", get_actions_param, 2);
    // communicator->send_request(get_actions_request);
    // auto actions = communicator->get_response<GetActionsResult>()->result.actions;

    // // Step environments
    // std::vector<std::vector<bool>> dones;
    // std::vector<std::vector<float>> rewards;
    // std::vector<std::future<std::unique_ptr<StepInfo>>> step_info_futures(environments.size());
    // for (unsigned int j = 0; j < environments.size(); ++j)
    // {
    //     std::vector<std::vector<int>> env_actions;
    //     for (int k = 0; k < agents_per_env; ++k)
    //     {
    //         env_actions.push_back(actions[(j * agents_per_env) + k]);
    //     }
    //     step_info_futures[j] = environments[j]->step(env_actions, 1. / 60.);
    // }
    // for (unsigned int i = 0; i < environments.size(); ++i)
    // {
    //     std::unique_ptr<StepInfo> step_info = step_info_futures[i].get();
    //     observations[i] = step_info->observation;
    //     dones.push_back(step_info->done);
    //     rewards.push_back(step_info->reward);
    //     for (unsigned int j = 0; j < step_info->reward.size(); ++j)
    //     {
    //         env_scores[i] += step_info->reward[j];
    //         if (step_info->done[j])
    //         {
    //             score_processor->add_score(0, env_scores[i]);
    //             env_scores[i] = 0;
    //         }
    //     }
    // }
    // if (action_frame_counter % 1000 == 0)
    // {
    //     std::vector<float> score_averages = score_processor->get_scores();
    //     spdlog::info("Average reward: {}", fmt::join(score_averages, " "));
    // }

    // // Send result to training server
    // std::shared_ptr<GiveRewardsParam> give_rewards_param = std::make_shared<GiveRewardsParam>();
    // give_rewards_param->rewards = interleave_vectors<float>(rewards);
    // give_rewards_param->dones = interleave_vectors<bool>(dones);
    // give_rewards_param->session_id = 0;
    // Request<GiveRewardsParam> give_rewards_request("give_rewards", give_rewards_param, 3);
    // communicator->send_request<GiveRewardsParam>(give_rewards_request);
    // if (action_frame_counter % 512 != 0)
    // {
    //     communicator->get_response<GiveRewardsResult>();
    // }
    // else
    // {
    //     waiting = true;
    //     std::thread response_thread = std::thread([this]() {
    //         communicator->get_response<GiveRewardsResult>();
    //         waiting = false;
    //     });
    //     response_thread.detach();
    // }
}
}