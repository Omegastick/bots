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
  private:
    int laser_count;
    float fov;
    float laser_length;
    std::vector<float> last_reading;

  public:
    LaserSensorModule(int laser_count = 9, float fov = 180, float laser_length = 20);

    virtual std::vector<float> get_sensor_reading();
    virtual RenderData get_render_data(bool lightweight = false);
    virtual nlohmann::json to_json() const;
};
}