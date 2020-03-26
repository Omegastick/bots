#pragma once

#include <entt/entity/entity.hpp>

namespace ai
{
struct Sensor
{
    unsigned int count = 0;
    entt::entity first = entt::null;
};

struct SensorReading
{
    float value = 0.f;
    entt::entity next = entt::null;
};
}