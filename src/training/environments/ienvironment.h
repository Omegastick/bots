#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <Box2D/Box2D.h>
#include <torch/torch.h>

#include "graphics/idrawable.h"
#include "misc/random.h"
#include "training/events/ievent.h"

namespace SingularityTrainer
{
class Body;
class IEffect;
class IEntity;
class Particle;
class RewardConfig;

typedef std::tuple<float, float, float> Transform;

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
             int tick)
        : agent_transforms(agent_transforms),
          entity_states(entity_states),
          tick(tick) {}

    EnvState(std::vector<b2Transform> &&agent_transforms,
             std::unordered_map<unsigned int, b2Transform> &&entity_states,
             int tick)
        : agent_transforms(agent_transforms),
          entity_states(entity_states),
          tick(tick) {}

    EnvState(std::vector<Transform> &agent_transforms,
             std::unordered_map<unsigned int, Transform> &entity_transforms, int tick)
    {
        std::transform(agent_transforms.begin(), agent_transforms.end(),
                       std::back_inserter(this->agent_transforms),
                       [](const Transform &transform) {
                           return b2Transform{{std::get<0>(transform), std::get<1>(transform)},
                                              b2Rot(std::get<2>(transform))};
                       });

        for (const auto &pair : entity_transforms)
        {
            this->entity_states[pair.first] = b2Transform({std::get<0>(pair.second), std::get<1>(pair.second)},
                                                          b2Rot(std::get<2>(pair.second)));
        }

        this->tick = tick;
    }

    std::vector<b2Transform> agent_transforms;
    std::unordered_map<unsigned int, b2Transform> entity_states;
    int tick;
};

struct StepInfo
{
    std::vector<std::unique_ptr<IEvent>> events;
    std::vector<torch::Tensor> observation;
    torch::Tensor reward, done;
    int victor = -1;
};

class IEnvironment : public IDrawable
{
  public:
    virtual ~IEnvironment() = 0;

    virtual void add_effect(std::unique_ptr<IEffect> effect) = 0;
    virtual void add_entity(std::unique_ptr<IEntity> entity) = 0;
    virtual void add_event(std::unique_ptr<IEvent> event) = 0;
    virtual void change_reward(int body, float reward_delta) = 0;
    virtual void change_reward(Body *body, float reward_delta) = 0;
    virtual void forward(float step_length) = 0;
    virtual std::vector<Body *> get_bodies() = 0;
    virtual float get_elapsed_time() const = 0;
    virtual std::unordered_map<unsigned int, std::unique_ptr<IEntity>> &get_entities() = 0;
    virtual RenderData get_render_data(bool lightweight = false) = 0;
    virtual RewardConfig &get_reward_config() = 0;
    virtual std::vector<float> get_scores() = 0;
    virtual b2World &get_world() = 0;
    virtual StepInfo reset() = 0;
    virtual void set_done() = 0;
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
}