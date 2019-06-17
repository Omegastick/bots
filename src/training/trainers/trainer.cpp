#include <memory>
#include <chrono>

#include <Box2D/Box2D.h>
#include <cpprl/cpprl.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>
#include <torch/torch.h>

#include "trainer.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "training/agents/agent.h"
#include "training/environments/ienvironment.h"
#include "training/score_processor.h"
#include "training/training_program.h"
#include "misc/utilities.h"
#include "misc/date.h"

namespace SingularityTrainer
{
const int agent_per_env = 2;
const int game_length = 600;
const bool recurrent = false;

Trainer::Trainer(TrainingProgram program,
                 AgentFactory &agent_factory,
                 IEnvironmentFactory &env_factory)
    : action_frame_counter(0),
      agents_per_env(2),
      elapsed_time(0),
      env_count(program.hyper_parameters.num_env),
      frame_counter(0),
      last_update_time(std::chrono::high_resolution_clock::now()),
      policy(nullptr),
      program(program),
      waiting(false)
{
    torch::manual_seed(0);

    // Initialize rollout storage
    b2World b2_world({0, 0});
    Random rng(0);
    auto temp_agent = agent_factory.make(b2_world, rng);
    temp_agent->load_json(program.agent);
    int num_observations = temp_agent->get_observation().size();
    int num_inputs = temp_agent->get_input_count();
    rollout_storage = std::make_unique<cpprl::RolloutStorage>(game_length,
                                                              program.hyper_parameters.num_env,
                                                              c10::IntArrayRef{num_observations},
                                                              cpprl::ActionSpace{"MultiBinary", {num_inputs}},
                                                              24,
                                                              torch::kCPU);

    for (int i = 0; i < env_count; ++i)
    {
        auto world = std::make_unique<b2World>(b2Vec2_zero);
        auto rng = std::make_unique<Random>(i);
        std::vector<std::unique_ptr<Agent>> agents;
        agents.push_back(agent_factory.make(*world, *rng));
        agents.push_back(agent_factory.make(*world, *rng));
        environments.push_back(env_factory.make(std::move(rng), std::move(world), std::move(agents)));
    }
    env_scores.resize(env_count);

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

    nn_base = std::make_shared<cpprl::MlpBase>(num_observations, recurrent, 24);
    policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {4}}, nn_base);
    algorithm = std::make_unique<cpprl::PPO>(policy,
                                             program.hyper_parameters.clip_param,
                                             program.hyper_parameters.num_epoch,
                                             program.hyper_parameters.num_minibatch,
                                             program.hyper_parameters.actor_loss_coef,
                                             program.hyper_parameters.value_loss_coef,
                                             program.hyper_parameters.entropy_coef,
                                             program.hyper_parameters.learning_rate);

    rollout_storage->set_first_observation(observations);
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
    } while (std::chrono::high_resolution_clock::now() - clock_start < std::chrono::duration<double>(1. / 60.));
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

void Trainer::save_model()
{
    auto date_time_string = date::format("%F-%H-%M", std::chrono::system_clock::now());
    torch::save(policy, date_time_string + ".pth");
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
                env_scores[i] = 0;
            }
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
        rollout_storage->compute_returns(next_value, true, 0.99, 0.95);

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
        auto update_end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> update_duration = update_end_time - update_start_time;
        spdlog::info("Update took {:.2f}s", update_duration.count());

        last_update_time = update_end_time;
    }
}
}