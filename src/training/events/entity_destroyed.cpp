#include <tuple>

#include <spdlog/spdlog.h>

#include "entity_destroyed.h"
#include "training/entities/ientity.h"
#include "training/environments/ienvironment.h"
#include "training/events/ievent.h"

namespace SingularityTrainer
{
EntityDestroyed::EntityDestroyed(unsigned int entity_id, double time, Transform transform)
    : entity_id(entity_id),
      time(time),
      transform(transform)
{
    type = EventTypes::EntityDestroyed;
}

void EntityDestroyed::trigger(IEnvironment &env)
{
    auto &entities = env.get_entities();
    auto iter = entities.find(entity_id);
    if (iter == entities.end())
    {
        spdlog::warn("Entity {} not found in client environment",
                     entity_id);
        return;
    }

    auto &entity = *iter->second;
    entity.set_transform({transform.get_position().x, transform.get_position().y},
                         transform.get_rotation());
    entity.destroy();
}
}