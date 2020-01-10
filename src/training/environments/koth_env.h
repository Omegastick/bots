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
#include "training/effects/ieffect.h"
#include "training/entities/bullet.h"
#include "training/entities/hill.h"
#include "training/entities/wall.h"
#include "training/entities/ientity.h"
#include "training/environments/ienvironment.h"
#include "training/training_program.h"

namespace ai
{
class IAudioEngine;
class IEvent;
struct Particle;
class Renderer;
class ResourceManager;

class KothEnv : public IEnvironment
{
  private:
    bool audible;
    IAudioEngine &audio_engine;
    std::unique_ptr<Body> body_1;
    std::unique_ptr<Body> body_2;
    IBulletFactory &bullet_factory;
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
            RewardConfig reward_config,
            IAudioEngine &audio_engine,
            IBulletFactory &bullet_factory);

    virtual void add_effect(std::unique_ptr<IEffect> effect);
    virtual void add_entity(std::unique_ptr<IEntity> entity);
    virtual void add_event(std::unique_ptr<IEvent> event);
    virtual void set_state(const EnvState &state);
    virtual void clear_effects();
    virtual StepInfo step(std::vector<torch::Tensor> actions, float step_length);
    virtual void forward(float step_length);
    virtual StepInfo reset();
    virtual void change_reward(int body, float reward_delta);
    virtual void change_reward(Body *body, float reward_delta);
    virtual void set_done();
    virtual void draw(Renderer &renderer, bool lightweight = false);
    virtual double get_elapsed_time() const;

    inline std::vector<Body *> get_bodies() { return {body_1.get(), body_2.get()}; }
    inline std::unordered_map<unsigned int, std::unique_ptr<IEntity>> &get_entities()
    {
        return entities;
    }
    inline RewardConfig &get_reward_config() { return reward_config; }
    inline Random &get_rng() { return *rng; }
    inline std::vector<float> get_scores() const { return scores; }
    inline b2World &get_world() { return *world; };
    inline bool is_audible() const { return audible; }
    inline void set_body_1(std::unique_ptr<Body> body) { this->body_1 = std::move(body); }
    inline void set_body_2(std::unique_ptr<Body> body) { this->body_2 = std::move(body); }
    inline void set_elapsed_time(double elapsed_time) { this->elapsed_time = elapsed_time; }
    inline void set_audibility(bool visibility) { audible = visibility; }
};

static auto MaxSteps = [] {};

class KothEnvFactory : public IEnvironmentFactory
{
  private:
    IAudioEngine &audio_engine;
    BodyFactory &body_factory;
    IBulletFactory &bullet_factory;
    int max_steps;

  public:
    BOOST_DI_INJECT(KothEnvFactory,
                    (named = MaxSteps) int max_steps,
                    IAudioEngine &audio_engine,
                    BodyFactory &body_factory,
                    IBulletFactory &bullet_factory)
        : audio_engine(audio_engine),
          body_factory(body_factory),
          bullet_factory(bullet_factory),
          max_steps(max_steps) {}

    int get_num_bodies() override;
    std::unique_ptr<IEnvironment> make() override;
    std::unique_ptr<IEnvironment> make(std::unique_ptr<Random> rng,
                                       std::unique_ptr<b2World> world,
                                       std::vector<std::unique_ptr<Body>> bodies,
                                       RewardConfig reward_config) override;
};
}