#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <utility>

#include "resource_manager.h"
#include "training/agents/iagent.h"
#include "training/environments/ienvironment.h"
#include "training/entities/wall.h"

namespace SingularityTrainer
{
class TargetEnv : IEnvironment
{
  public:
    TargetEnv(ResourceManager &resource_manager, float x, float y, float scale, int max_steps);
    ~TargetEnv();

    virtual void start_thread();
    virtual std::future<std::unique_ptr<StepInfo>> step(std::vector<int> &actions, float step_length);
    virtual void forward(float step_length);
    virtual std::future<std::unique_ptr<StepInfo>> reset();
    virtual void change_reward(float reward_delta);
    virtual void set_done();
    virtual void draw(sf::RenderTarget &render_target);

  private:
    sf::RenderTexture render_texture;
    sf::Sprite sprite;
    std::unique_ptr<b2World> world;
    std::unique_ptr<IAgent> agent;
    std::vector<std::unique_ptr<Wall>> walls;
    std::unique_ptr<b2ContactListener> contact_listener;
    bool done;
    float reward;
    int step_counter;
    const int max_steps;
    std::thread *thread;
    std::queue<ThreadCommand> command_queue;
    std::atomic<int> command_queue_flag;

    void thread_loop();
};
}