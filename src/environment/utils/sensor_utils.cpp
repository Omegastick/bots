#include <doctest.h>
#include <entt/entt.hpp>

#include "environment/components/sensor_reading.h"
#include "environment/systems/clean_up_system.h"

namespace ai
{
void resize_sensor(entt::registry &registry, entt::entity sensor_entity, unsigned int size)
{
    auto &sensor = registry.get<Sensor>(sensor_entity);
    entt::entity sensor_reading_entity = sensor.first;
    for (unsigned int i = 0; i < sensor.count; i++)
    {
        registry.assign_or_replace<entt::tag<"should_destroy"_hs>>(sensor_reading_entity);
        sensor_reading_entity = registry.get<SensorReading>(sensor_reading_entity).next;
    }

    if (size == 0)
    {
        return;
    }

    sensor_reading_entity = registry.create();
    sensor.count = size;
    sensor.first = sensor_reading_entity;
    registry.assign<SensorReading>(sensor_reading_entity);
    for (unsigned int i = 0; i < sensor.count - 1; i++)
    {
        const auto temp_entity = registry.create();
        registry.assign<SensorReading>(temp_entity);
        registry.get<SensorReading>(sensor_reading_entity).next = temp_entity;
        sensor_reading_entity = temp_entity;
    }
}

TEST_CASE("resize_sensor()")
{
    entt::registry registry;

    const auto sensor_entity = registry.create();
    registry.assign<Sensor>(sensor_entity);

    SUBCASE("Correctly initializes sensors")
    {
        resize_sensor(registry, sensor_entity, 5);

        const auto view = registry.view<SensorReading>();
        DOCTEST_CHECK(view.size() == 5);
    }

    SUBCASE("Correctly resizes sensors")
    {
        resize_sensor(registry, sensor_entity, 5);
        resize_sensor(registry, sensor_entity, 8);

        clean_up_system(registry);
        const auto view = registry.view<SensorReading>();
        DOCTEST_CHECK(view.size() == 8);
    }

    SUBCASE("Correctly resizes to 0")
    {
        resize_sensor(registry, sensor_entity, 5);
        resize_sensor(registry, sensor_entity, 0);
        clean_up_system(registry);
        const auto view = registry.view<SensorReading>();
        DOCTEST_CHECK(view.size() == 0);
    }
}
}