#pragma once

#include <condition_variable>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <utility>
#include <mutex>

#include <Box2D/Box2D.h>
#include <torch/torch.h>

#include "training/environments/ienvironment.h"

namespace ai
{
class Random;
class ResourceManager;
class Body;
class Target;
class Wall;

class TargetEnv : public IEnvironment
{
  private:
    int max_steps;
    std::unique_ptr<b2World> world;
    std::vector<std::unique_ptr<Wall>> walls;
    std::unique_ptr<Target> target;
    std::unique_ptr<b2ContactListener> contact_listener;
    float reward;
    int step_counter;
    bool done;
    std::thread *thread;
    std::queue<ThreadCommand> command_queue;
    std::mutex command_queue_mutex;
    std::condition_variable command_queue_condvar;
    int total_reward;
    std::unique_ptr<Random> rng;
    float elapsed_time;

    void thread_loop();
    void reset_impl();

  public:
    TargetEnv(float x, float y, float scale, int max_steps, int seed);
    ~TargetEnv();

    virtual void start_thread();
    virtual std::future<StepInfo> step(torch::Tensor actions, float step_length);
    virtual void forward(float step_length);
    virtual std::future<StepInfo> reset();
    virtual void change_reward(int body, float reward_delta);
    virtual void change_reward(Body *body, float reward_delta);
    virtual void set_done();
    void draw(Renderer &renderer, bool lightweight = false);
    virtual double get_elapsed_time() const;

    std::unique_ptr<Body> body;
};
}