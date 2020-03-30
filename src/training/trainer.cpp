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
#include "environment/iecs_env.h"
#include "environment/ecs_env.h"
#include "graphics/colors.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/utils/range.h"
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

namespace ai
{
const bool recurrent = false;

Trainer::Trainer(std::unique_ptr<NNAgent> agent,
                 std::unique_ptr<cpprl::Algorithm> algorithm,
                 std::unique_ptr<std::vector<std::unique_ptr<IAgent>>> opponent_pool,
                 TrainingProgram program,
                 std::unique_ptr<MultiRolloutGenerator> rollout_generator,
                 Checkpointer &checkpointer,
                 EloEvaluator &evaluator)
    : agent(std::move(agent)),
      algorithm(std::move(algorithm)),
      checkpointer(checkpointer),
      env_count(program.hyper_parameters.num_env),
      evaluator(evaluator),
      last_save_time(std::chrono::high_resolution_clock::now()),
      last_update_time(std::chrono::high_resolution_clock::now()),
      new_opponents(1 + program.opponent_pool.size()),
      opponent_pool(std::move(opponent_pool)),
      previous_checkpoint(program.checkpoint),
      program(program),
      reset_recently(true),
      returns_rms(1),
      rollout_generator(std::move(rollout_generator)),
      skip_update(false) {}

void Trainer::draw(Renderer &renderer, bool lightweight)
{
    rollout_generator->draw(renderer, lightweight);
}

double Trainer::evaluate()
{
    spdlog::debug("Evaluating agent");
    std::vector<IAgent *> new_opponents_vec;
    for (unsigned int i = opponent_pool->size() - new_opponents; i < opponent_pool->size(); ++i)
    {
        new_opponents_vec.push_back((*opponent_pool)[i].get());
    }
    new_opponents = 0;
    return evaluator.evaluate(*agent, new_opponents_vec, 80);
}

std::vector<std::pair<std::string, float>> Trainer::step_batch()
{
    auto rollout = rollout_generator->generate();
    if (skip_update)
    {
        return {};
    }
    auto update_data = learn(rollout);
    return update_data;
}

std::filesystem::path Trainer::save_model(std::filesystem::path directory)
{
    spdlog::debug("Saving model");
    previous_checkpoint = checkpointer.save(agent->get_policy(),
                                            program.body,
                                            {},
                                            previous_checkpoint,
                                            directory);
    spdlog::debug("Model saved to: {}", previous_checkpoint.string());

    return previous_checkpoint;
}

std::vector<std::pair<std::string, float>> Trainer::learn(cpprl::RolloutStorage &rollout)
{
    auto update_start_time = std::chrono::high_resolution_clock::now();

    torch::Tensor next_value;
    {
        torch::NoGradGuard no_grad;
        next_value = agent->get_policy()->get_values(
                                            rollout.get_observations()[-1],
                                            rollout.get_hidden_states()[-1],
                                            rollout.get_masks()[-1])
                         .detach();
    }
    // Divide rewards by return variance
    rollout.compute_returns(next_value,
                            false,
                            program.hyper_parameters.discount_factor,
                            0.95f);
    returns_rms->update(rollout.get_returns());
    rollout.set_rewards(torch::clamp(
        rollout.get_rewards() / (returns_rms->get_variance().sqrt() + 1e-8),
        -10,
        10));

    // Calculate the returns for real this time
    rollout.compute_returns(next_value,
                            true,
                            program.hyper_parameters.discount_factor,
                            0.95f);

    auto update_data = algorithm->update(rollout);
    rollout.after_update();

    spdlog::info("---");
    spdlog::info("Total frames: {}", rollout_generator->get_timestep());
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
        opponent_pool->push_back(std::make_unique<NNAgent>(
            agent->get_policy(),
            program.body,
            date::format("%F-%H-%M", std::chrono::system_clock::now())));
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
    update_pairs.push_back({"Total Frames", rollout_generator->get_timestep()});
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

void Trainer::stop()
{
    skip_update = true;
    rollout_generator->stop();
}

std::unique_ptr<Trainer> TrainerFactory::make(TrainingProgram &program) const
{
    torch::manual_seed(0);

    const unsigned int num_observations = program.body["num_observations"];
    const unsigned int num_actions = program.body["num_actions"];

    // Initialize opponent pool
    auto opponent_pool = std::make_unique<std::vector<std::unique_ptr<IAgent>>>();
    opponent_pool->push_back(std::make_unique<RandomAgent>(program.body, rng, "Random Agent"));
    for (const auto &checkpoint_path : program.opponent_pool)
    {
        auto policy = checkpointer.load(checkpoint_path).policy;
        opponent_pool->push_back(std::make_unique<NNAgent>(policy, program.body, checkpoint_path));
    }

    // Initialize environments
    std::vector<std::unique_ptr<IEcsEnv>> environments;
    for (int i = 0; i < program.hyper_parameters.num_env; i++)
    {
        environments.push_back(std::make_unique<EcsEnv>());
    }

    cpprl::Policy policy(nullptr);
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

    auto agent = std::make_unique<NNAgent>(policy, program.body, "Agent");

    std::vector<std::unique_ptr<ISingleRolloutGenerator>> sub_generators;
    for (int i = 0; i < program.hyper_parameters.num_env; i++)
    {
        sub_generators.push_back(single_rollout_generator_factory.make(
            *agent,
            std::move(environments[i]),
            *opponent_pool));
    }

    auto rollout_generator = std::make_unique<MultiRolloutGenerator>(
        program.hyper_parameters.batch_size,
        std::move(sub_generators));

    std::unique_ptr<cpprl::Algorithm> algorithm;
    if (program.hyper_parameters.algorithm == Algorithm::A2C)
    {
        algorithm = std::make_unique<cpprl::A2C>(agent->get_policy(),
                                                 program.hyper_parameters.actor_loss_coef,
                                                 program.hyper_parameters.value_loss_coef,
                                                 program.hyper_parameters.entropy_coef,
                                                 program.hyper_parameters.learning_rate);
    }
    else if (program.hyper_parameters.algorithm == Algorithm::PPO)
    {
        algorithm = std::make_unique<cpprl::PPO>(agent->get_policy(),
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

    return std::make_unique<Trainer>(std::move(agent),
                                     std::move(algorithm),
                                     std::move(opponent_pool),
                                     program,
                                     std::move(rollout_generator),
                                     checkpointer,
                                     evaluator);
}
}