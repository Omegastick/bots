#include <tuple>

#include <spdlog/spdlog.h>

#include "effect_triggered.h"
#include "graphics/colors.h"
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
            cl_white));
    }
    else
    {
        spdlog::warn("Effect {} not implemented in EffectTriggered", effect_type);
    }
}
}