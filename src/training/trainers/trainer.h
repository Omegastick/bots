#pragma once

#include <chrono>
#include <vector>

#include <cpprl/model/policy.h>
#include <cpprl/storage.h>
#include <torch/torch.h>

#include "third_party/di.hpp"
#include "training/agents/agent.h"
#include "training/environments/ienvironment.h"
#include "training/trainers/itrainer.h"
#include "training/training_program.h"

namespace cpprl
{
class Algorithm;
class NNBase;
}

namespace SingularityTrainer
{
class IEnvironmentFactory;

class Trainer : public ITrainer
{
  private:
    std::unique_ptr<Agent> example_agent;

    int action_frame_counter;
    int agents_per_env;
    std::unique_ptr<cpprl::Algorithm> algorithm;
    float elapsed_time;
    int env_count;
    std::vector<float> env_scores;
    int frame_counter;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_update_time;
    std::shared_ptr<cpprl::NNBase> nn_base;
    torch::Tensor observations;
    cpprl::Policy policy;
    std::unique_ptr<cpprl::RolloutStorage> rollout_storage;
    TrainingProgram program;
    bool waiting;

    void action_update();

  public:
    Trainer(TrainingProgram program,
            AgentFactory &agent_factory,
            IEnvironmentFactory &env_factory);

    virtual void save_model();
    virtual void step();
    virtual void slow_step();
};

class TrainerFactory
{
  private:
    AgentFactory &agent_factory;
    IEnvironmentFactory &env_factory;

  public:
    TrainerFactory(AgentFactory &agent_factory, IEnvironmentFactory &env_factory)
        : agent_factory(agent_factory), env_factory(env_factory) {}

    std::unique_ptr<Trainer> make(TrainingProgram &program)
    {
        return std::make_unique<Trainer>(program, agent_factory, env_factory);
    }
};
}