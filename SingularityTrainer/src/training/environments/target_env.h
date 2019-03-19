#pragma once

#include <Box2D/Box2D.h>
#include <atomic>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <utility>
#include <mutex>

#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
class Random;
class ResourceManager;
class Agent;
class Target;
class Wall;

class TargetEnv : public IEnvironment
{
  private:
    std::unique_ptr<b2World> world;
    std::vector<std::unique_ptr<Wall>> walls;
    std::unique_ptr<Target> target;
    std::unique_ptr<b2ContactListener> contact_listener;
    bool done;
    float reward;
    int step_counter;
    const int max_steps;
    std::thread *thread;
    std::queue<ThreadCommand> command_queue;
    std::atomic<int> command_queue_flag;
    std::mutex command_queue_mutex;
    int total_reward;
    std::unique_ptr<Random> rng;
    float elapsed_time;

    void thread_loop();

  public:
    TargetEnv(float x, float y, float scale, int max_steps, int seed);
    ~TargetEnv();

    virtual void start_thread();
    virtual std::future<std::unique_ptr<StepInfo>> step(const std::vector<std::vector<int>> &actions, float step_length);
    virtual void forward(float step_length);
    virtual std::future<std::unique_ptr<StepInfo>> reset();
    virtual void change_reward(int agent, float reward_delta);
    virtual void set_done();
    virtual RenderData get_render_data(bool lightweight = false);
    virtual float get_elapsed_time() const;

    std::unique_ptr<Agent> agent;
};
}