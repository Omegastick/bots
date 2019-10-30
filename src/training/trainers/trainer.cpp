#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <stdexcept>

#include <Box2D/Box2D.h>
#include <cpprl/cpprl.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <taskflow/taskflow.hpp>
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
#include "training/evaluators/elo_evaluator.h"
#include "training/score_processor.h"
#include "training/training_program.h"
#include "third_party/date.h"
#include "misc/random.h"
#include "misc/utilities.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
const bool recurrent = false;

Trainer::Trainer(TrainingProgram program,
                 BodyFactory &body_factory,
                 Checkpointer &checkpointer,
                 IEnvironmentFactory &env_factory,
                 EloEvaluator &evaluator,
                 Random &rng)
    : batch_number(0),
      checkpointer(checkpointer),
      env_count(program.hyper_parameters.num_env),
      evaluator(evaluator),
      last_save_time(std::chrono::high_resolution_clock::now()),
      last_update_time(std::chrono::high_resolution_clock::now()),
      new_opponents(1 + program.opponent_pool.size()),
      policy(nullptr),
      previous_checkpoint(program.checkpoint),
      program(program),
      reset_recently(true),
      returns_rms(1),
      rng(rng),
      slow(false),
      timestep(0)
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

    // Initialize env mutexes
    std::vector<std::mutex> temp_mutex_vec(program.hyper_parameters.num_env);
    env_mutexes.swap(temp_mutex_vec);

    // Initialize opponent pool
    opponent_pool.push_back(std::make_unique<RandomAgent>(program.body, rng, "Random Agent"));
    for (const auto &checkpoint_path : program.opponent_pool)
    {
        auto policy = checkpointer.load(checkpoint_path).policy;
        opponent_pool.push_back(std::make_unique<NNAgent>(policy, program.body, checkpoint_path));
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

    auto observations = torch::zeros({env_count, num_observations});
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
                torch::rand({1, num_actions}).round(),
                torch::rand({1, opponents[i]->get_action_size()}).round()};
            env_observation = environments[i]->step(actions, 1.f / 10.f).observation;
            if (current_step == (i * 12) - 1)
            {
                observations[i] = env_observation[0];
                opponent_observations[i] = env_observation[1];
            }
        }
    }

    if (program.checkpoint.empty())
    {
        spdlog::debug("Making new agent");
        auto nn_base = std::make_shared<cpprl::MlpBase>(num_observations, recurrent);
        policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {num_actions}}, nn_base, true);
    }
    else
    {
        spdlog::debug("Loading {}", program.checkpoint);
        auto checkpoint = checkpointer.load(program.checkpoint);
        policy = checkpoint.policy;
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

void Trainer::draw(Renderer &renderer, bool lightweight)
{
    {
        std::lock_guard lock_guard(env_mutexes[0]);
        environments[0]->draw(renderer, lightweight);
    }

    for (unsigned int i = 1; i < environments.size(); ++i)
    {
        std::lock_guard lock_guard(env_mutexes[i]);
        environments[i]->clear_effects();
    }
}

double Trainer::evaluate()
{
    spdlog::debug("Evaluating agent");
    std::vector<IAgent *> new_opponents_vec;
    for (unsigned int i = opponent_pool.size() - new_opponents; i < opponent_pool.size(); ++i)
    {
        new_opponents_vec.push_back(opponent_pool[i].get());
    }
    new_opponents = 0;
    NNAgent agent(policy, program.body, "Current Agent");
    return evaluator.evaluate(agent, new_opponents_vec, 80);
}

std::vector<std::pair<std::string, float>> Trainer::step_batch()
{
    spdlog::debug("Starting new training batch");
    tf::Executor executor;
    tf::Taskflow task_flow;

    std::vector<cpprl::RolloutStorage> storages;

    for (int i = 0; i < program.hyper_parameters.num_env; ++i)
    {
        storages.push_back({program.hyper_parameters.batch_size,
                            1,
                            c10::IntArrayRef{program.body["num_observations"]},
                            cpprl::ActionSpace{"MultiBinary", {program.body["num_actions"]}},
                            64,
                            torch::kCPU});
        storages[i].set_first_observation(rollout_storage->get_observations()[0][i]);
    }

    for (int i = 0; i < program.hyper_parameters.num_env; ++i)
    {
        tf::Task last_task;
        for (int step = 0; step < program.hyper_parameters.batch_size; ++step)
        {
            tf::Task task = task_flow.emplace([this, &storages, step, i] {
                // Get action from policy
                std::vector<torch::Tensor> act_result;
                {
                    torch::NoGradGuard no_grad;
                    act_result = policy->act(storages[i].get_observations()[step],
                                             storages[i].get_hidden_states()[step],
                                             storages[i].get_masks()[step]);
                }

                // Get opponent action
                auto opponent_act_result = opponents[i]->act(opponent_observations[i],
                                                             opponent_hidden_states[i],
                                                             opponent_masks[i]);
                opponent_hidden_states[i] = std::get<1>(opponent_act_result);

                // Step environment
                torch::Tensor dones = torch::zeros({1, 1});
                torch::Tensor opponent_dones = torch::zeros({1, 1});
                torch::Tensor rewards = torch::zeros({1, 1});

                StepInfo step_info;
                {
                    std::lock_guard lock_guard(env_mutexes[i]);
                    step_info = environments[i]->step({act_result[1], std::get<0>(opponent_act_result)},
                                                      1.f / 60.f);
                }
                if (slow)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
                }
                for (int mini_step = 0; mini_step < 5; ++mini_step)
                {
                    {
                        std::lock_guard lock_guard(env_mutexes[i]);
                        environments[i]->forward(1.f / 60.f);
                    }
                    if (slow)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
                    }
                }
                dones = step_info.done[0];
                rewards = step_info.reward[0];
                opponent_observations[i] = step_info.observation[1];
                opponent_dones = step_info.done[1];
                env_scores[i] += step_info.reward[0].item().toFloat();
                if (step_info.done[0].item().toBool())
                {
                    if (i == 0)
                    {
                        reset_recently = true;
                    }
                    env_scores[i] = 0;
                    environments[i]->reset();
                    int selected_opponent = rng.next_int(0, opponent_pool.size());
                    opponents[i] = opponent_pool[selected_opponent].get();
                    opponent_hidden_states[i] = torch::zeros({opponents[i]->get_hidden_state_size(), 1});
                }
                storages[i].insert(step_info.observation[0],
                                   act_result[3],
                                   act_result[1],
                                   act_result[2],
                                   act_result[0],
                                   rewards,
                                   1 - dones);

                ++timestep;
            });

            if (step != 0)
            {
                last_task.precede(task);
            }
            last_task = task;
        }
    }

    spdlog::debug("Gathering training data");
    executor.run(task_flow);
    executor.wait_for_all();

    batch_number++;

    spdlog::debug("Processing training data");
    std::vector<cpprl::RolloutStorage *> storage_ptrs;
    std::transform(storages.begin(), storages.end(), std::back_inserter(storage_ptrs),
                   [](cpprl::RolloutStorage &storage) { return &storage; });
    rollout_storage = std::make_unique<cpprl::RolloutStorage>(storage_ptrs, torch::kCPU);

    spdlog::debug("Learning from training data");
    auto update_data = learn();

    spdlog::debug("Training batch finished");
    return update_data;
}

std::filesystem::path Trainer::save_model(std::filesystem::path directory)
{
    spdlog::debug("Saving model");
    previous_checkpoint = checkpointer.save(policy,
                                            program.body,
                                            {},
                                            previous_checkpoint,
                                            directory);
    spdlog::debug("Model saved to: {}", previous_checkpoint.string());

    return previous_checkpoint;
}

std::vector<std::pair<std::string, float>> Trainer::learn()
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
    // Divide rewards by return variance
    rollout_storage->compute_returns(next_value,
                                     false,
                                     program.hyper_parameters.discount_factor,
                                     0.95f);
    returns_rms->update(rollout_storage->get_returns());
    rollout_storage->set_rewards(torch::clamp(
        rollout_storage->get_rewards() / (returns_rms->get_variance().sqrt() + 1e-8),
        -10,
        10));

    // Calculate the returns for real this time
    rollout_storage->compute_returns(next_value,
                                     true,
                                     program.hyper_parameters.discount_factor,
                                     0.95f);

    auto update_data = algorithm->update(*rollout_storage);
    rollout_storage->after_update();

    spdlog::info("---");
    spdlog::info("Total frames: {}", timestep);
    const std::chrono::duration<double> sim_duration = update_start_time - last_update_time;
    const double fps = (env_count * program.hyper_parameters.batch_size) / sim_duration.count();
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
        opponent_pool.push_back(std::make_unique<NNAgent>(
            policy,
            program.body,
            date::format("%F-%H-%M-%S", std::chrono::system_clock::now())));
        new_opponents++;
        last_save_time = now;
    }

    std::vector<std::pair<std::string, float>> update_pairs;
    std::transform(update_data.begin(), update_data.end(),
                   std::back_inserter(update_pairs),
                   [](const cpprl::UpdateDatum &datum) {
                       return std::pair<std::string, float>{datum.name, datum.value};
                   });
    update_pairs.push_back({"FPS", fps});
    update_pairs.push_back({"Total Frames", timestep});
    update_pairs.push_back({"Update Duration", update_duration.count()});

    return update_pairs;
}

bool Trainer::should_clear_particles()
{
    if (reset_recently)
    {
        reset_recently = false;
        return true;
    }
    return false;
}
}