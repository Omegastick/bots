#include <tuple>

#include <fmt/format.h>
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
        return;
        // auto error_string = fmt::format("Entity {} not found in client environment",
        //                                 entity_id);
        // throw std::runtime_error(error_string);
    }

    auto &entity = *iter->second;
    entity.set_transform({std::get<0>(transform), std::get<1>(transform)}, std::get<2>(transform));
    entity.destroy();
}
}