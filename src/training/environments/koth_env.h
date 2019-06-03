#pragma once

#include <Box2D/Box2D.h>
#include <atomic>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <utility>
#include <mutex>
#include <unordered_map>

#include "misc/random.h"
#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
class ResourceManager;
class Agent;
class Wall;
class Hill;

class KothEnv : public IEnvironment
{
  private:
    int max_steps;
    Random rng;
    b2World world;
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

    void thread_loop();
    void reset_impl();

  public:
    KothEnv(int max_steps, Random &&rng);
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

    std::unique_ptr<Agent> agent_1;
    std::unique_ptr<Agent> agent_2;
};

class KothEnvFactory : public IEnvironmentFactory
{
  private:
    int max_steps;

  public:
    KothEnvFactory(int max_steps) : max_steps(max_steps) {}

    std::unique_ptr<IEnvironment> make(int seed)
    {
        return std::make_unique<KothEnv>(max_steps, Random(seed));
    }
};
}