#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "environment/components/modules/base_module.h"
#include "environment/components/modules/module.h"
#include "environment/components/physics_body.h"
#include "environment/components/sensor_reading.h"
#include "environment/utils/body_factories.h"

namespace ai
{
void base_module_system(entt::registry &registry)
{
    const auto view = registry.view<EcsBaseModule>();
    for (const auto entity : view)
    {
        const auto &module = registry.get<EcsModule>(entity);
        const auto &physics_body = registry.get<PhysicsBody>(module.body).body;
        auto linear_velocity = physics_body->GetLinearVelocity();
        linear_velocity = b2Mul(physics_body->GetTransform().q, linear_velocity);
        const auto angular_velocity = physics_body->GetAngularVelocity();

        const auto &sensor = registry.get<Sensor>(entity);
        auto *sensor_reading = &registry.get<SensorReading>(sensor.first);
        sensor_reading->value = linear_velocity.x;
        sensor_reading = &registry.get<SensorReading>(sensor_reading->next);
        sensor_reading->value = linear_velocity.y;
        sensor_reading = &registry.get<SensorReading>(sensor_reading->next);
        sensor_reading->value = angular_velocity;
    }
}

TEST_CASE("base_module_system()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    auto &physics_body = registry.get<PhysicsBody>(body_entity);

    SUBCASE("Stores velocities in sensor readings")
    {
        physics_body.body->SetLinearVelocity({1, 2});
        physics_body.body->SetAngularVelocity(3);

        base_module_system(registry);

        auto entity = registry.view<SensorReading>().back();
        DOCTEST_CHECK(registry.get<SensorReading>(entity).value == doctest::Approx(1.f));
        entity = registry.get<SensorReading>(entity).next;
        DOCTEST_CHECK(registry.get<SensorReading>(entity).value == doctest::Approx(2.f));
        entity = registry.get<SensorReading>(entity).next;
        DOCTEST_CHECK(registry.get<SensorReading>(entity).value == doctest::Approx(3.f));
    }

    SUBCASE("Velocities are correct when rotated")
    {
        physics_body.body->SetLinearVelocity({1, 2});
        physics_body.body->SetAngularVelocity(3);
        physics_body.body->SetTransform({0, 0}, glm::radians(90.f));

        base_module_system(registry);

        auto entity = registry.view<SensorReading>().back();
        DOCTEST_CHECK(registry.get<SensorReading>(entity).value == doctest::Approx(-2.f));
        entity = registry.get<SensorReading>(entity).next;
        DOCTEST_CHECK(registry.get<SensorReading>(entity).value == doctest::Approx(1.f));
        entity = registry.get<SensorReading>(entity).next;
        DOCTEST_CHECK(registry.get<SensorReading>(entity).value == doctest::Approx(3.f));
    }
}
}