#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <Box2D/Box2D.h>
#include <torch/torch.h>
#include <trompeloeil.hpp>

#include "graphics/renderers/renderer.h"
#include "misc/random.h"
#include "misc/transform.h"
#include "training/events/ievent.h"

namespace SingularityTrainer
{
class Body;
class IEffect;
class IEntity;
struct Particle;
struct RewardConfig;

struct EntityState
{
    unsigned int id;
    b2Transform transform;
};

struct EnvState
{
    EnvState() {}

    EnvState(std::vector<b2Transform> &agent_transforms,
             std::unordered_map<unsigned int, b2Transform> &entity_states,
             std::vector<float> hps,
             std::vector<float> scores,
             int tick)
        : agent_transforms(agent_transforms),
          entity_states(entity_states),
          hps(hps),
          scores(scores),
          tick(tick) {}

    EnvState(std::vector<b2Transform> &&agent_transforms,
             std::unordered_map<unsigned int, b2Transform> &&entity_states,
             std::vector<float> hps,
             std::vector<float> scores,
             int tick)
        : agent_transforms(agent_transforms),
          entity_states(entity_states),
          hps(hps),
          scores(scores),
          tick(tick) {}

    EnvState(std::vector<Transform> &agent_transforms,
             std::unordered_map<unsigned int, Transform> &entity_transforms,
             std::vector<float> hps,
             std::vector<float> scores,
             int tick)
        : hps(hps),
          scores(scores),
          tick(tick)
    {
        std::transform(agent_transforms.begin(), agent_transforms.end(),
                       std::back_inserter(this->agent_transforms),
                       [](const Transform &transform) {
                           return b2Transform({transform.get_position().x,
                                               transform.get_position().y},
                                              b2Rot(transform.get_rotation()));
                       });

        for (const auto &pair : entity_transforms)
        {
            this->entity_states[pair.first] = b2Transform({pair.second.get_position().x,
                                                           pair.second.get_position().y},
                                                          b2Rot(pair.second.get_rotation()));
        }
    }

    std::vector<b2Transform> agent_transforms;
    std::unordered_map<unsigned int, b2Transform> entity_states;
    std::vector<float> hps;
    std::vector<float> scores;
    int tick;
};

struct StepInfo
{
    std::vector<std::unique_ptr<IEvent>> events;
    std::vector<torch::Tensor> observation;
    torch::Tensor reward, done;
    int victor = -1;
};

class IEnvironment
{
  public:
    virtual ~IEnvironment() = 0;

    virtual void add_effect(std::unique_ptr<IEffect> effect) = 0;
    virtual void add_entity(std::unique_ptr<IEntity> entity) = 0;
    virtual void add_event(std::unique_ptr<IEvent> event) = 0;
    virtual void change_reward(int body, float reward_delta) = 0;
    virtual void change_reward(Body *body, float reward_delta) = 0;
    virtual void clear_effects() = 0;
    virtual void forward(float step_length) = 0;
    virtual std::vector<Body *> get_bodies() = 0;
    virtual double get_elapsed_time() const = 0;
    virtual std::unordered_map<unsigned int, std::unique_ptr<IEntity>> &get_entities() = 0;
    virtual void draw(Renderer &renderer, bool lightweight = false) = 0;
    virtual RewardConfig &get_reward_config() = 0;
    virtual std::vector<float> get_scores() const = 0;
    virtual b2World &get_world() = 0;
    virtual StepInfo reset() = 0;
    virtual void set_done() = 0;
    virtual void set_elapsed_time(double elapsed_time) = 0;
    virtual void set_state(const EnvState &state) = 0;
    virtual StepInfo step(std::vector<torch::Tensor> actions, float step_length) = 0;
};

inline IEnvironment::~IEnvironment() {}

class IEnvironmentFactory
{
  public:
    virtual ~IEnvironmentFactory() = 0;

    virtual int get_num_bodies() = 0;
    virtual std::unique_ptr<IEnvironment> make(std::unique_ptr<Random> rng,
                                               std::unique_ptr<b2World> world,
                                               std::vector<std::unique_ptr<Body>> bodies,
                                               RewardConfig reward_config) = 0;
    virtual std::unique_ptr<IEnvironment> make() = 0;
};

inline IEnvironmentFactory::~IEnvironmentFactory() {}

class MockEnvironment : public trompeloeil::mock_interface<IEnvironment>
{
  public:
    IMPLEMENT_MOCK1(add_effect);
    IMPLEMENT_MOCK1(add_entity);
    IMPLEMENT_MOCK1(add_event);
    MAKE_MOCK2(change_reward, void(int, float));
    MAKE_MOCK2(change_reward, void(Body *, float));
    IMPLEMENT_MOCK0(clear_effects);
    IMPLEMENT_MOCK1(forward);
    IMPLEMENT_MOCK0(get_bodies);
    IMPLEMENT_CONST_MOCK0(get_elapsed_time);
    IMPLEMENT_MOCK0(get_entities);
    IMPLEMENT_MOCK2(draw);
    IMPLEMENT_MOCK0(get_reward_config);
    IMPLEMENT_CONST_MOCK0(get_scores);
    IMPLEMENT_MOCK0(get_world);
    IMPLEMENT_MOCK0(reset);
    IMPLEMENT_MOCK0(set_done);
    IMPLEMENT_MOCK1(set_elapsed_time);
    IMPLEMENT_MOCK1(set_state);
    IMPLEMENT_MOCK2(step);
};
}