#pragma once

#include <Box2D/Box2D.h>
#include <atomic>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <utility>

#include "graphics/render_data.h"
#include "graphics/idrawable.h"
#include "random.h"
#include "resource_manager.h"
#include "training/agents/iagent.h"
#include "training/entities/target.h"
#include "training/entities/wall.h"
#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
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
    int total_reward;
    Random rng;
    float elapsed_time;

    void thread_loop();

  public:
    TargetEnv(float x, float y, float scale, int max_steps, int seed);
    ~TargetEnv();

    virtual void start_thread();
    virtual std::future<std::unique_ptr<StepInfo>> step(std::vector<int> &actions, float step_length);
    virtual void forward(float step_length);
    virtual std::future<std::unique_ptr<StepInfo>> reset();
    virtual void change_reward(float reward_delta);
    virtual void set_done();
    virtual RenderData get_render_data(bool lightweight = false);
    virtual float get_elapsed_time() const;

    std::unique_ptr<IAgent> agent;
};
}