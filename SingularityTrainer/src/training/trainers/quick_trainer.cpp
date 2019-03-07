#include <memory>
#include <chrono>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include "communicator.h"
#include "random.h"
#include "resource_manager.h"
#include "training/environments/target_env.h"
#include "training/environments/koth_env.h"
#include "training/trainers/quick_trainer.h"
#include "utilities.h"
#include "date.h"

namespace SingularityTrainer
{
QuickTrainer::QuickTrainer(Communicator *communicator, Random *rng, int env_count)
    : communicator(communicator),
      waiting_for_server(false),
      env_count(env_count),
      frame_counter(0),
      action_frame_counter(0),
      rng(rng),
      elapsed_time(0),
      score_processor(1, 0.99),
      agents_per_env(2)
{
    for (int i = 0; i < env_count; ++i)
    {
        environments.push_back(std::make_unique<KothEnv>(460, 40, 1, 100, i));
    }
    env_scores.resize(env_count);
}

QuickTrainer::~QuickTrainer() {}

void QuickTrainer::begin_training()
{
    std::vector<std::future<std::unique_ptr<StepInfo>>> observation_futures(env_count);
    observations.resize(env_count);
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        environments[i]->start_thread();
        observation_futures[i] = environments[i]->reset();

        // Start each environment with a different number of random steps to decorrelate the environments
        for (unsigned int current_step = 0; current_step < i * 10; current_step++)
        {
            std::vector<int> actions;
            actions.reserve(4);
            for (int action = 0; action < 4; action++)
            {
                actions.push_back(rng->next_int(0, 1));
            }
            observation_futures[i] = environments[i]->step({actions, actions}, 1.f / 10.f);
        }
    }
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        observations[i] = observation_futures[i].get()->observation;
    }

    // Init training session
    Model model{12, 4, true, true};
    HyperParams hyperparams;
    hyperparams.learning_rate = 0.0007;
    hyperparams.batch_size = 512;
    hyperparams.num_minibatch = env_count;
    hyperparams.epochs = 3;
    hyperparams.discount_factor = 0.99;
    hyperparams.use_gae = true;
    hyperparams.gae = 0.95;
    hyperparams.critic_coef = 0.5;
    hyperparams.entropy_coef = 0.01;
    hyperparams.max_grad_norm = 0.5;
    hyperparams.clip_factor = 0.2;
    hyperparams.use_gpu = false;
    hyperparams.normalize_rewards = true;
    std::shared_ptr<BeginSessionParam> begin_session_param = std::make_shared<BeginSessionParam>();
    begin_session_param->session_id = 0;
    begin_session_param->model = std::move(model);
    begin_session_param->hyperparams = std::move(hyperparams);
    begin_session_param->contexts = env_count * agents_per_env;
    begin_session_param->auto_train = true;
    begin_session_param->training = true;
    begin_session_param->session_id = 0;
    Request<BeginSessionParam> begin_session_request("begin_session", begin_session_param, 1);
    communicator->send_request<BeginSessionParam>(begin_session_request);
    communicator->get_response<BeginSessionResult>();
}

void QuickTrainer::end_training()
{
    std::shared_ptr<EndSessionParam> end_session_param = std::make_shared<EndSessionParam>();
    end_session_param->session_id = 0;
    Request<EndSessionParam> end_session_request("end_session", end_session_param, 4);
    communicator->send_request<EndSessionParam>(end_session_request);
    communicator->get_response<EndSessionResult>();
}

void QuickTrainer::step()
{
    if (waiting_for_server)
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
        if (waiting_for_server)
        {
            break;
        }
    } while (std::chrono::high_resolution_clock::now() - clock_start < std::chrono::duration<double>(1. / 60.));
}

void QuickTrainer::slow_step()
{
    if (waiting_for_server)
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
    auto date_time_string = date::format("%F_%H:%M", std::chrono::system_clock::now());
    auto save_model_param = std::make_shared<SaveModelParam>();
    save_model_param->path = "./" + date_time_string + ".pth";
    save_model_param->session_id = 0;
    Request<SaveModelParam> save_model_request("save_model", save_model_param, 0);
    communicator->send_request(save_model_request);
    communicator->get_response<SaveModelResult>();
}

void QuickTrainer::action_update()
{
    action_frame_counter++;

    // Get actions from training server
    std::shared_ptr<GetActionsParam> get_actions_param = std::make_shared<GetActionsParam>();
    get_actions_param->inputs = interleave_vectors<std::vector<float>>(observations);
    get_actions_param->session_id = 0;
    Request<GetActionsParam> get_actions_request("get_actions", get_actions_param, 2);
    communicator->send_request(get_actions_request);
    auto actions = communicator->get_response<GetActionsResult>()->result.actions;

    // Step environments
    std::vector<std::vector<bool>> dones;
    std::vector<std::vector<float>> rewards;
    std::vector<std::future<std::unique_ptr<StepInfo>>> step_info_futures(environments.size());
    for (unsigned int j = 0; j < environments.size(); ++j)
    {
        std::vector<std::vector<int>> env_actions;
        for (int k = 0; k < agents_per_env; ++k)
        {
            env_actions.push_back(actions[(j * agents_per_env) + k]);
        }
        step_info_futures[j] = environments[j]->step(env_actions, 1. / 60.);
    }
    for (unsigned int i = 0; i < environments.size(); ++i)
    {
        std::unique_ptr<StepInfo> step_info = step_info_futures[i].get();
        observations[i] = step_info->observation;
        dones.push_back(step_info->done);
        rewards.push_back(step_info->reward);
        for (int j = 0; j < step_info->reward.size(); ++j)
        {
            env_scores[i] += step_info->reward[j];
            if (step_info->done[j])
            {
                score_processor.add_score(0, env_scores[i]);
                env_scores[i] = 0;
            }
        }
    }
    if (action_frame_counter % 100 == 0)
    {
        std::vector<float> score_averages = score_processor.get_scores();
        spdlog::info("Average reward: {}", fmt::join(score_averages, " "));
    }

    // Send result to training server
    std::shared_ptr<GiveRewardsParam> give_rewards_param = std::make_shared<GiveRewardsParam>();
    give_rewards_param->rewards = interleave_vectors<float>(rewards);
    give_rewards_param->dones = interleave_vectors<bool>(dones);
    give_rewards_param->session_id = 0;
    Request<GiveRewardsParam> give_rewards_request("give_rewards", give_rewards_param, 3);
    communicator->send_request<GiveRewardsParam>(give_rewards_request);
    if (action_frame_counter % 512 != 0)
    {
        communicator->get_response<GiveRewardsResult>();
    }
    else
    {
        waiting_for_server = true;
        std::thread response_thread = std::thread([this]() {
            communicator->get_response<GiveRewardsResult>();
            waiting_for_server = false;
        });
        response_thread.detach();
    }
}
}