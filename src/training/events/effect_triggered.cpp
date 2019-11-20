#include <tuple>

#include <spdlog/spdlog.h>

#include "effect_triggered.h"
#include "graphics/colors.h"
#include "training/effects/body_hit.h"
#include "training/effects/thruster_particles.h"
#include "training/entities/ientity.h"
#include "training/environments/ienvironment.h"
#include "training/events/ievent.h"

namespace SingularityTrainer
{
EffectTriggered::EffectTriggered(EffectTypes effect_type, double time, Transform transform)
    : effect_type(effect_type),
      time(time),
      transform(transform)
{
    type = EventTypes::EffectTriggered;
}

void EffectTriggered::trigger(IEnvironment &env)
{
    if (effect_type == EffectTypes::ThrusterParticles)
    {
        env.add_effect(std::make_unique<ThrusterParticles>(
            b2Transform({transform.get_position().x, transform.get_position().y},
                        b2Rot(transform.get_rotation())),
            glm::vec4{1.23f, 0.53f, 0.28f, 1.f}));
    }
    else if (effect_type == EffectTypes::BodyHit)
    {
        env.add_effect(std::make_unique<BodyHit>(
            b2Vec2(transform.get_position().x, transform.get_position().y),
            cl_white));
    }
    else
    {
        spdlog::warn("Effect {} not implemented in EffectTriggered", effect_type);
    }
}
}