#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>

#include "training/modules/imodule.h"

namespace SingularityTrainer
{
class RenderData;

class ClosestRaycastCallback : public b2RayCastCallback
{
  public:
    ClosestRaycastCallback();
    ~ClosestRaycastCallback();

    virtual float32 ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float32 fraction);

    float distance;
};

class LaserSensorModule : public IModule
{
  public:
    LaserSensorModule();

    virtual std::vector<float> get_sensor_reading();
    virtual RenderData get_render_data(bool lightweight = false);

    int laser_count;
    float laser_length;
    float fov;
    std::vector<float> last_reading;
};
}