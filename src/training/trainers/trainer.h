#pragma once

#include <chrono>
#include <filesystem>
#include <vector>

#include <cpprl/algorithms/algorithm.h>
#include <cpprl/model/nn_base.h>
#include <cpprl/model/policy.h>
#include <cpprl/storage.h>
#include <torch/torch.h>

#include "third_party/di.hpp"
#include "training/agents/iagent.h"
#include "training/bodies/body.h"
#include "training/trainers/itrainer.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
class Checkpointer;
class EloEvaluator;
class IEnvironmentFactory;
class Random;

class Trainer : public ITrainer
{
  private:
    std::unique_ptr<Body> example_body;

    int action_frame_counter;
    std::unique_ptr<cpprl::Algorithm> algorithm;
    Checkpointer &checkpointer;
    float elapsed_time;
    int env_count;
    std::vector<float> env_scores;
    EloEvaluator &evaluator;
    int frame_counter;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_save_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update_time;
    int new_opponents;
    torch::Tensor observations;
    std::vector<torch::Tensor> opponent_hidden_states;
    std::vector<torch::Tensor> opponent_observations;
    torch::Tensor opponent_masks;
    std::vector<std::unique_ptr<IAgent>> opponent_pool;
    std::vector<IAgent *> opponents;
    cpprl::Policy policy;
    std::filesystem::path previous_checkpoint;
    TrainingProgram program;
    Random &rng;
    std::unique_ptr<cpprl::RolloutStorage> rollout_storage;
    bool waiting;

    void action_update();

  public:
    Trainer(TrainingProgram program,
            BodyFactory &body_factory,
            Checkpointer &checkpointer,
            IEnvironmentFactory &env_factory,
            EloEvaluator &evaluator,
            Random &rng);

    virtual float evaluate();
    virtual std::vector<float> get_observation();
    virtual std::filesystem::path save_model(std::filesystem::path directory = {});
    virtual void step();
    virtual void slow_step();
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