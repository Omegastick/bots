#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "environment/components/bullet.h"
#include "environment/components/hill.h"
#include "environment/components/modules/laser_sensor_module.h"
#include "environment/components/sensor_reading.h"
#include "environment/utils/body_factories.h"
#include "graphics/render_data.h"
#include "misc/transform.h"

namespace ai
{
constexpr float fov = 180.f;
constexpr float laser_length = 20.f;

class ClosestRaycastCallback : public b2RayCastCallback
{
  private:
    entt::registry &registry;

  public:
    ClosestRaycastCallback(entt::registry &registry) : registry(registry) {}

    virtual float32 ReportFixture(b2Fixture *fixture,
                                  const b2Vec2 & /*point*/,
                                  const b2Vec2 & /*normal*/,
                                  float32 fraction)
    {
        const auto entity = static_cast<entt::registry::entity_type>(
            reinterpret_cast<uintptr_t>(fixture->GetUserData()));
        if (registry.has<EcsBullet>(entity) || registry.has<EcsHill>(entity))
        {
            return 1;
        }
        distance = fraction;
        return fraction;
    }

    float distance = 1.f;
};

template <typename Functor>
void cast_lasers(entt::registry &registry, const entt::entity &module_entity, Functor &&callback)
{
    auto &module = registry.get<EcsLaserSensorModule>(module_entity);
    auto transform = static_cast<b2Transform>(registry.get<Transform>(module_entity));

    float segment_width = fov / static_cast<float>(module.laser_count - 1);

    for (unsigned int i = 0; i < module.laser_count; ++i)
    {
        ClosestRaycastCallback raycast_callback(registry);

        b2Rot angle(glm::radians((segment_width * static_cast<float>(i)) - (fov * 0.5f)));
        b2Vec2 laser = b2Mul(angle, b2Vec2(0, laser_length));

        registry.ctx<b2World>().RayCast(&raycast_callback,
                                        transform.p,
                                        b2Mul(transform, laser));

        callback(raycast_callback.distance);
    }
}

void laser_sensor_module_system(entt::registry &registry)
{
    const auto view = registry.view<EcsLaserSensorModule>();
    for (const auto &entity : view)
    {
        const auto &sensor = registry.get<Sensor>(entity);
        entt::entity sensor_reading_entity = sensor.first;

        cast_lasers(registry, entity, [&](float distance) {
            auto &sensor_reading = registry.get<SensorReading>(sensor_reading_entity);
            if (distance == -1)
            {
                sensor_reading.value = 1;
            }
            else
            {
                sensor_reading.value = distance;
            }
            sensor_reading_entity = sensor_reading.next;
        });
    }
}

void draw_lasers_system(entt::registry &registry)
{
    const auto laser_view = registry.view<entt::tag<"laser"_hs>>();
    for (const auto &entity : laser_view)
    {
        registry.assign_or_replace<entt::tag<"should_destroy"_hs>>(entity);
    }

    const auto view = registry.view<EcsLaserSensorModule>();
    for (const auto &module_entity : view)
    {
        const auto &module = registry.get<EcsLaserSensorModule>(module_entity);
        float segment_width = fov / static_cast<float>(module.laser_count - 1);

        int i = 0;
        cast_lasers(registry, module_entity, [&](float distance) {
            if (distance >= 1)
            {
                i++;
                return;
            }

            const auto laser_entity = registry.create();
            registry.assign<entt::tag<"laser"_hs>>(laser_entity);

            auto transform = static_cast<b2Transform>(registry.get<Transform>(module_entity));

            b2Rot angle(glm::radians((segment_width * static_cast<float>(i++)) - (fov * 0.5f)));
            b2Vec2 laser = b2Mul(angle, b2Vec2(0, distance * laser_length));
            b2Vec2 laser_start = b2Mul(angle, b2Vec2(0, 0.35f));
            b2Vec2 transformed_end = b2Mul(transform, laser);
            b2Vec2 transformed_start = b2Mul(transform, laser_start);
            registry.assign<Line>(laser_entity,
                                  glm::vec2{transformed_start.x, transformed_start.y},
                                  glm::vec2{transformed_end.x, transformed_end.y},
                                  set_alpha(cl_white, 0.5f),
                                  0.02f);
        });
    }
}

TEST_CASE("laser_sensor_module_system()")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    make_laser_sensor_module(registry);

    laser_sensor_module_system(registry);

    SUBCASE("Stores distances in sensor readings")
    {
        registry.view<SensorReading>().each([&](const auto &sensor_reading) {
            DOCTEST_CHECK(sensor_reading.value == 1.f);
        });
    }
}
}