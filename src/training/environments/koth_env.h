#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <unordered_map>

#include <Box2D/Box2D.h>

#include "misc/random.h"
#include "third_party/di.hpp"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"
#include "training/environments/ienvironment.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
class IEffect;
class IEntity;
class IEvent;
class Hill;
class Particle;
class ResourceManager;
class Wall;

class KothEnv : public IEnvironment
{
  private:
    std::unique_ptr<Body> body_1;
    std::unique_ptr<Body> body_2;
    std::vector<std::unique_ptr<IEffect>> effects;
    std::unordered_map<unsigned int, std::unique_ptr<IEntity>> entities;
    std::vector<std::unique_ptr<IEvent>> events;
    int max_steps;
    std::unique_ptr<Random> rng;
    std::unique_ptr<b2World> world;
    std::vector<std::unique_ptr<Wall>> walls;
    std::unique_ptr<Hill> hill;
    std::unique_ptr<b2ContactListener> contact_listener;
    double elapsed_time;
    bool done;
    std::vector<float> rewards;
    std::vector<float> scores;
    int step_counter;
    std::unordered_map<const Body *, int> body_numbers;
    RewardConfig reward_config;

    void change_score(Body *, float score_delta);
    void change_score(int, float score_delta);

  public:
    KothEnv(int max_steps,
            std::unique_ptr<Body> body_1,
            std::unique_ptr<Body> body_2,
            std::unique_ptr<b2World> world,
            std::unique_ptr<Random> rng,
            RewardConfig reward_config);
    ~KothEnv();

    virtual void add_effect(std::unique_ptr<IEffect> effect);
    virtual void add_entity(std::unique_ptr<IEntity> entity);
    virtual void add_event(std::unique_ptr<IEvent> event);
    virtual void set_state(const EnvState &state);
    virtual StepInfo step(std::vector<torch::Tensor> actions, float step_length);
    virtual void forward(float step_length);
    virtual StepInfo reset();
    virtual void change_reward(int body, float reward_delta);
    virtual void change_reward(Body *body, float reward_delta);
    virtual void set_done();
    virtual RenderData get_render_data(bool lightweight = false);
    virtual double get_elapsed_time() const;

    inline std::vector<Body *> get_bodies() { return {body_1.get(), body_2.get()}; }
    inline std::unordered_map<unsigned int, std::unique_ptr<IEntity>> &get_entities() { return entities; }
    inline RewardConfig &get_reward_config() { return reward_config; }
    inline Random &get_rng() { return *rng; }
    inline std::vector<float> get_scores() { return scores; }
    inline b2World &get_world() { return *world; };
    inline void set_body_1(std::unique_ptr<Body> body) { this->body_1 = std::move(body); }
    inline void set_body_2(std::unique_ptr<Body> body) { this->body_2 = std::move(body); }
    inline void set_elapsed_time(double elapsed_time) { this->elapsed_time = elapsed_time; }
};

static auto MaxSteps = [] {};

class KothEnvFactory : public IEnvironmentFactory
{
  private:
    BodyFactory &body_factory;
    int max_steps;

  public:
    BOOST_DI_INJECT(KothEnvFactory,
                    (named = MaxSteps) int max_steps,
                    BodyFactory &body_factory)
        : body_factory(body_factory),
          max_steps(max_steps) {}

    virtual int get_num_bodies() { return 2; }
    virtual std::unique_ptr<IEnvironment> make(
        std::unique_ptr<Random> rng,
        std::unique_ptr<b2World> world,
        std::vector<std::unique_ptr<Body>> bodies,
        RewardConfig reward_config)
    {
        return std::make_unique<KothEnv>(max_steps,
                                         std::move(bodies[0]),
                                         std::move(bodies[1]),
                                         std::move(world),
                                         std::move(rng),
                                         reward_config);
    }

    virtual std::unique_ptr<IEnvironment> make()
    {
        auto rng = std::make_unique<Random>(0);
        auto world = std::make_unique<b2World>(b2Vec2(0, 0));
        return std::make_unique<KothEnv>(max_steps,
                                         body_factory.make(*world, *rng),
                                         body_factory.make(*world, *rng),
                                         std::move(world),
                                         std::move(rng),
                                         RewardConfig());
    }
};
}