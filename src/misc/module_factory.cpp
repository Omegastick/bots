#include <memory>
#include <stdexcept>
#include <string>

#include <fmt/format.h>

#include "module_factory.h"
#include "training/entities/bullet.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/imodule.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/square_hull.h"
#include "training/modules/thruster_module.h"

namespace ai
{
std::shared_ptr<IModule> ModuleFactory::create_module(const std::string &module_id)
{
    if (module_id == "base_module")
    {
        return std::make_shared<BaseModule>();
    }
    else if (module_id == "gun_module")
    {
        return std::make_shared<GunModule>(bullet_factory, rng);
    }
    else if (module_id == "laser_sensor_module")
    {
        return std::make_shared<LaserSensorModule>();
    }
    else if (module_id == "square_hull")
    {
        return std::make_shared<SquareHull>();
    }
    else if (module_id == "thruster_module")
    {
        return std::make_shared<ThrusterModule>();
    }

    throw std::runtime_error(fmt::format("No such module {}", module_id));
}
}