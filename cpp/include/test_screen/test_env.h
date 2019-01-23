#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <future>
#include <memory>
#include <queue>
#include <thread>
#include <utility>
#include <atomic>

#include "idrawable.h"
#include "test_screen/bot.h"
#include "test_screen/target.h"
#include "test_screen/wall.h"
#include "test_screen/score_display.h"

namespace SingularityTrainer
{
class Target;

struct StepInfo
{
    std::vector<float> observation;
    float reward;
    bool done;
};

class TestEnv : IDrawable
{
  public:
    TestEnv(ResourceManager &resource_manager, float x, float y, float scale, int max_steps);
    ~TestEnv();

    void start_thread();
    void draw(sf::RenderTarget &render_target);
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