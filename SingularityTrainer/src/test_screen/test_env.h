#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <utility>

#include "idrawable.h"
#include "linear_particle_system.h"
#include "test_screen/bot.h"
#include "test_screen/score_display.h"
#include "training/entities/target.h"
#include "training/entities/wall.h"
#include "training/environments/ienvironment.h"

namespace SingularityTrainer
{
class TestEnv : public IEnvironment
{
  public:
    TestEnv(ResourceManager &resource_manager, float x, float y, float scale, int max_steps);
    ~TestEnv();

    void start_thread();
    void draw(sf::RenderTarget &render_target, bool lightweight = false);
    std::future<std::unique_ptr<StepInfo>> step(std::vector<int> &actions, float step_length);
    void forward(float step_length);
    std::future<std::unique_ptr<StepInfo>> reset();
    void change_reward(float reward_delta);
    void set_done();

  private:
    enum Commands
    {
        Step,
        Forward,
        Reset,
        Quit
    };
    struct ThreadCommand
    {
        Commands command;
        std::promise<std::unique_ptr<StepInfo>> promise;
        float step_length;
        std::vector<int> actions;
    };
    sf::RenderTexture render_texture;
    sf::Sprite sprite;
    std::unique_ptr<b2World> world;
    std::unique_ptr<Bot> bot;
    std::unique_ptr<Target> target;
    std::vector<std::unique_ptr<Wall>> walls;
    std::unique_ptr<b2ContactListener> contact_listener;
    bool done;
    float reward;
    int step_counter;
    const int max_steps;
    std::thread *thread;
    std::queue<ThreadCommand> command_queue;
    std::atomic<int> command_queue_flag;
    ScoreDisplay score_display;

    void thread_loop();
};
}