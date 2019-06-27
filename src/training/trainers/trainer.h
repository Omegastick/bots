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
#include "training/bodies/body.h"
#include "training/trainers/itrainer.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
class Checkpointer;
class BasicEvaluator;
class IEnvironmentFactory;

class Trainer : public ITrainer
{
  private:
    std::unique_ptr<Body> example_body;

    int action_frame_counter;
    int bodies_per_env;
    std::unique_ptr<cpprl::Algorithm> algorithm;
    Checkpointer &checkpointer;
    float elapsed_time;
    int env_count;
    std::vector<float> env_scores;
    BasicEvaluator &evaluator;
    int frame_counter;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update_time;
    torch::Tensor observations;
    cpprl::Policy policy;
    std::filesystem::path previous_checkpoint;
    std::unique_ptr<cpprl::RolloutStorage> rollout_storage;
    TrainingProgram program;
    bool waiting;

    void action_update();

  public:
    Trainer(TrainingProgram program,
            BodyFactory &body_factory,
            Checkpointer &checkpointer,
            IEnvironmentFactory &env_factory,
            BasicEvaluator &evaluator);

    virtual float evaluate(int number_of_trials);
    virtual std::vector<float> get_observation();
    virtual std::filesystem::path save_model();
    virtual void step();
    virtual void slow_step();
};

class TrainerFactory
{
  private:
    BodyFactory &body_factory;
    Checkpointer &checkpointer;
    IEnvironmentFactory &env_factory;
    BasicEvaluator &evaluator;

  public:
    TrainerFactory(BodyFactory &body_factory,
                   Checkpointer &checkpointer,
                   IEnvironmentFactory &env_factory,
                   BasicEvaluator &evaluator)
        : body_factory(body_factory),
          checkpointer(checkpointer),
          env_factory(env_factory),
          evaluator(evaluator) {}

    std::unique_ptr<Trainer> make(TrainingProgram &program)
    {
        return std::make_unique<Trainer>(program, body_factory, checkpointer, env_factory, evaluator);
    }
};
}