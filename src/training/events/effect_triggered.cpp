#include <tuple>

#include <spdlog/spdlog.h>

#include "effect_triggered.h"
#include "graphics/colors.h"
#include "misc/random.h"
#include "training/effects/thruster_particles.h"
#include "training/entities/ientity.h"
#include "training/environments/ienvironment.h"
#include "training/events/ievent.h"

namespace SingularityTrainer
{
EffectTriggered::EffectTriggered(EffectTypes effect_type, double time, Transform transform, Random &rng)
    : effect_type(effect_type),
      rng(rng),
      time(time),
      transform(transform)
{
    type = EventTypes::EffectTriggered;
}

void EffectTriggered::trigger(IEnvironment &env)
{
    if (effect_type == EffectTypes::ThrusterParticles)
    {
        env.add_effect(std::make_unique<ThrusterParticles>(b2Transform({std::get<0>(transform),
                                                                        std::get<1>(transform)},
                                                                       b2Rot(std::get<2>(transform))),
                                                           cl_white,
                                                           rng));
    }
    else
    {
        spdlog::warn("Effect {} not implemented in EffectTriggered", effect_type);
    }
}
}