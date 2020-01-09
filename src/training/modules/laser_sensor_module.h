#pragma once

#include <memory>
#include <vector>

#include <Box2D/Box2D.h>

#include "graphics/render_data.h"
#include "training/modules/imodule.h"

namespace ai
{
class Renderer;

class ClosestRaycastCallback : public b2RayCastCallback
{
  public:
    ClosestRaycastCallback();
    ~ClosestRaycastCallback();

    virtual float32 ReportFixture(b2Fixture *fixture,
                                  const b2Vec2 &point,
                                  const b2Vec2 &normal,
                                  float32 fraction);

    float distance;
};

class LaserSensorModule : public IModule
{
  private:
    int laser_count;
    float fov;
    float laser_length;
    SemiCircle semi_circle;

    std::vector<float> cast_lasers() const;

  public:
    LaserSensorModule(int laser_count = 20, float fov = 180, float laser_length = 20);

    virtual void draw(Renderer &renderer, bool lightweight = false) override;
    virtual std::vector<float> get_sensor_reading() const override final;
    virtual void set_color(const ColorScheme &color_scheme) override;
    virtual nlohmann::json to_json() const override final;

    inline virtual int get_observation_count() const override final { return laser_count; }
};
}