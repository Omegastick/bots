#include <vector>

#include <entt/entt.hpp>
#include <torch/torch.h>

#include "environment/components/body.h"
#include "environment/components/sensor_reading.h"
#include "environment/utils/body_utils.h"

namespace ai
{
std::vector<torch::Tensor> observation_system(entt::registry &registry)
{
    const auto body_view = registry.view<EcsBody>();
    std::vector<torch::Tensor> output;
    output.reserve(body_view.size());
    for (const auto &body_entity : body_view)
    {
        std::vector<float> observation;

        traverse_modules(registry, body_entity, [&](auto module_entity) {
            if (!registry.has<Sensor>(module_entity))
            {
                return;
            }

            const auto &sensor = registry.get<Sensor>(module_entity);
            entt::entity sensor_reading_entity = sensor.first;
            for (unsigned int i = 0; i < sensor.count; i++)
            {
                const auto &sensor_reading = registry.get<SensorReading>(sensor_reading_entity);
                observation.push_back(sensor_reading.value);
                sensor_reading_entity = sensor_reading.next;
            }
        });

        output.push_back(torch::from_blob(observation.data(),
                                          {1, static_cast<long>(observation.size())})
                             .clone());
    }

    return output;
}
}