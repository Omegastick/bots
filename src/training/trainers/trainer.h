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
#include "training/agents/agent.h"
#include "training/trainers/itrainer.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
class Checkpointer;
class IEnvironmentFactory;

class Trainer : public ITrainer
{
  private:
    std::unique_ptr<Agent> example_agent;

    int action_frame_counter;
    int agents_per_env;
    std::unique_ptr<cpprl::Algorithm> algorithm;
    Checkpointer &checkpointer;
    float elapsed_time;
    int env_count;
    std::vector<float> env_scores;
    int frame_counter;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_save_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update_time;
    std::shared_ptr<cpprl::NNBase> nn_base;
    torch::Tensor observations;
    cpprl::Policy policy;
    std::filesystem::path previous_checkpoint;
    std::unique_ptr<cpprl::RolloutStorage> rollout_storage;
    TrainingProgram program;
    bool waiting;

    void action_update();

  public:
    Trainer(TrainingProgram program,
            AgentFactory &agent_factory,
            Checkpointer &checkpointer,
            IEnvironmentFactory &env_factory);

    virtual std::vector<float> get_observation();
    virtual void save_model();
    virtual void step();
    virtual void slow_step();
};

class TrainerFactory
{
  private:
    AgentFactory &agent_factory;
    Checkpointer &checkpointer;
    IEnvironmentFactory &env_factory;

  public:
    TrainerFactory(AgentFactory &agent_factory,
                   Checkpointer &checkpointer,
                   IEnvironmentFactory &env_factory)
        : agent_factory(agent_factory),
          checkpointer(checkpointer),
          env_factory(env_factory) {}

    std::unique_ptr<Trainer> make(TrainingProgram &program)
    {
        return std::make_unique<Trainer>(program, agent_factory, checkpointer, env_factory);
    }
};
}