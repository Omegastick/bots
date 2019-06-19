#pragma once

#include <atomic>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <utility>
#include <mutex>
#include <unordered_map>

#include <Box2D/Box2D.h>

#include "misc/random.h"
#include "third_party/di.hpp"
#include "training/agents/test_agent.h"
#include "training/environments/ienvironment.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
class Agent;
class Hill;
class ResourceManager;
class Wall;

class KothEnv : public IEnvironment
{
  private:
    std::unique_ptr<Agent> agent_1;
    std::unique_ptr<Agent> agent_2;
    int max_steps;
    std::unique_ptr<Random> rng;
    std::unique_ptr<b2World> world;
    std::vector<std::unique_ptr<Wall>> walls;
    std::unique_ptr<Hill> hill;
    std::unique_ptr<b2ContactListener> contact_listener;
    float elapsed_time;
    bool done;
    std::vector<float> rewards;
    int step_counter;
    std::unique_ptr<std::thread> thread;
    std::queue<ThreadCommand> command_queue;
    std::mutex command_queue_mutex;
    std::condition_variable command_queue_condvar;
    std::vector<float> total_rewards;
    std::unordered_map<Agent *, int> agent_numbers;
    RewardConfig reward_config;

    void
    thread_loop();
    void reset_impl();

  public:
    KothEnv(int max_steps,
            std::unique_ptr<Agent> agent_1,
            std::unique_ptr<Agent> agent_2,
            std::unique_ptr<b2World> world,
            std::unique_ptr<Random> rng,
            RewardConfig reward_config);
    ~KothEnv();

    virtual void start_thread();
    virtual std::future<StepInfo> step(torch::Tensor actions, float step_length);
    virtual void forward(float step_length);
    virtual std::future<StepInfo> reset();
    virtual void change_reward(int agent, float reward_delta);
    virtual void change_reward(Agent *agent, float reward_delta);
    virtual void set_done();
    virtual RenderData get_render_data(bool lightweight = false);
    virtual float get_elapsed_time() const;

    inline std::vector<Agent *> get_agents() { return {agent_1.get(), agent_2.get()}; }
    inline RewardConfig &get_reward_config() { return reward_config; }
    inline Random &get_rng() { return *rng; }
    inline std::vector<float> get_total_rewards() { return total_rewards; }
    inline b2World &get_world() { return *world; };
    inline void set_agent_1(std::unique_ptr<Agent> agent) { this->agent_1 = std::move(agent); }
    inline void set_agent_2(std::unique_ptr<Agent> agent) { this->agent_2 = std::move(agent); }
};

auto MaxSteps = [] {};

class KothEnvFactory : public IEnvironmentFactory
{
  private:
    int max_steps;

  public:
    BOOST_DI_INJECT(KothEnvFactory, (named = MaxSteps) int max_steps)
        : max_steps(max_steps) {}

    virtual std::unique_ptr<IEnvironment> make(
        std::unique_ptr<Random> rng,
        std::unique_ptr<b2World> world,
        std::vector<std::unique_ptr<Agent>> agents,
        RewardConfig reward_config)
    {
        return std::make_unique<KothEnv>(max_steps,
                                         std::move(agents[0]),
                                         std::move(agents[1]),
                                         std::move(world),
                                         std::move(rng),
                                         reward_config);
    }
};
}