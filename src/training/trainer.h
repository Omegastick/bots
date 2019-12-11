#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include <cpprl/algorithms/algorithm.h>
#include <cpprl/running_mean_std.h>
#include <cpprl/storage.h>
#include <torch/torch.h>

#include "graphics/render_data.h"
#include "third_party/di.hpp"
#include "training/agents/iagent.h"
#include "training/agents/nn_agent.h"
#include "training/rollout_generators/multi_rollout_generator.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
class BodyFactory;
class Checkpointer;
class EloEvaluator;
class IEnvironmentFactory;
class Random;

class Trainer
{
  private:
    std::unique_ptr<NNAgent> agent;
    std::unique_ptr<cpprl::Algorithm> algorithm;
    Checkpointer &checkpointer;
    int env_count;
    EloEvaluator &evaluator;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_save_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update_time;
    int new_opponents;
    std::unique_ptr<std::vector<std::unique_ptr<IAgent>>> opponent_pool;
    std::filesystem::path previous_checkpoint;
    TrainingProgram program;
    bool reset_recently;
    cpprl::RunningMeanStd returns_rms;
    Random &rng;
    std::unique_ptr<MultiRolloutGenerator> rollout_generator;

    std::vector<std::pair<std::string, float>> learn(cpprl::RolloutStorage &rollout);

  public:
    Trainer(std::unique_ptr<NNAgent> agent,
            std::unique_ptr<cpprl::Algorithm> algorithm,
            std::unique_ptr<std::vector<std::unique_ptr<IAgent>>> opponent_pool,
            TrainingProgram program,
            std::unique_ptr<MultiRolloutGenerator> rollout_generator,
            Checkpointer &checkpointer,
            EloEvaluator &evaluator,
            Random &rng);

    void draw(Renderer &renderer, bool lightweight = false);
    double evaluate();
    std::filesystem::path save_model(std::filesystem::path directory = {});
    std::vector<std::pair<std::string, float>> step_batch();
    bool should_clear_particles();

    inline unsigned int get_batch_number() const { return rollout_generator->get_batch_number(); }
    inline std::string get_current_opponent() const
    {
        return rollout_generator->get_current_opponent(0);
    }
    inline std::vector<std::vector<float>> get_scores() const
    {
        return rollout_generator->get_scores();
    }
    inline unsigned long long get_timestep() const { return rollout_generator->get_timestep(); }
    inline const TrainingProgram &get_training_program() const { return program; }
    inline void set_fast() { rollout_generator->set_fast(); }
    inline void set_slow() { rollout_generator->set_slow(); }
};

class TrainerFactory
{
  private:
    BodyFactory &body_factory;
    Checkpointer &checkpointer;
    IEnvironmentFactory &env_factory;
    EloEvaluator &evaluator;
    Random &rng;

  public:
    TrainerFactory(BodyFactory &body_factory,
                   Checkpointer &checkpointer,
                   IEnvironmentFactory &env_factory,
                   EloEvaluator &evaluator,
                   Random &rng)
        : body_factory(body_factory),
          checkpointer(checkpointer),
          env_factory(env_factory),
          evaluator(evaluator),
          rng(rng) {}

    std::unique_ptr<Trainer> make(TrainingProgram &program) const;
};
}