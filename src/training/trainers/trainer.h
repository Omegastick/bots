#pragma once

#include <chrono>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

#include <cpprl/algorithms/algorithm.h>
#include <cpprl/model/nn_base.h>
#include <cpprl/model/policy.h>
#include <cpprl/running_mean_std.h>
#include <cpprl/storage.h>
#include <torch/torch.h>

#include "graphics/render_data.h"
#include "third_party/di.hpp"
#include "training/agents/iagent.h"
#include "training/environments/ienvironment.h"
#include "training/bodies/body.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
class Checkpointer;
class EloEvaluator;
class IEnvironmentFactory;
class Random;

class Trainer
{
  private:
    std::unique_ptr<Body> example_body;

    std::unique_ptr<cpprl::Algorithm> algorithm;
    int batch_number;
    Checkpointer &checkpointer;
    int env_count;
    std::vector<std::mutex> env_mutexes;
    std::vector<float> env_scores;
    std::vector<std::unique_ptr<IEnvironment>> environments;
    EloEvaluator &evaluator;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_save_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update_time;
    int new_opponents;
    std::vector<torch::Tensor> opponent_hidden_states;
    std::vector<torch::Tensor> opponent_observations;
    torch::Tensor opponent_masks;
    std::vector<std::unique_ptr<IAgent>> opponent_pool;
    std::vector<IAgent *> opponents;
    cpprl::Policy policy;
    std::filesystem::path previous_checkpoint;
    TrainingProgram program;
    cpprl::RunningMeanStd returns_rms;
    Random &rng;
    std::unique_ptr<cpprl::RolloutStorage> rollout_storage;
    bool slow;

    std::vector<std::pair<std::string, float>> learn();

  public:
    Trainer(TrainingProgram program,
            BodyFactory &body_factory,
            Checkpointer &checkpointer,
            IEnvironmentFactory &env_factory,
            EloEvaluator &evaluator,
            Random &rng);

    float evaluate();
    RenderData get_render_data(bool lightweight = false);
    std::filesystem::path save_model(std::filesystem::path directory = {});
    std::vector<std::pair<std::string, float>> step_batch();

    std::vector<std::unique_ptr<IEnvironment>> &get_environments() { return environments; }
    inline void set_fast() { slow = false; }
    inline void set_slow() { slow = true; }
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

    std::unique_ptr<Trainer> make(TrainingProgram &program)
    {
        return std::make_unique<Trainer>(program, body_factory, checkpointer, env_factory, evaluator, rng);
    }
};
}