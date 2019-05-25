#include <memory>
#include <string>

#include "misc/module_factory.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/imodule.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/thruster_module.h"

namespace SingularityTrainer
{
std::shared_ptr<IModule> ModuleFactory::create_module(std::string &module_id)
{
    if (module_id == "base")
    {
        return std::make_shared<BaseModule>();
    }
    else if (module_id == "gun")
    {
        return std::make_shared<GunModule>();
    }
    else if (module_id == "laser_sensor")
    {
        return std::make_shared<LaserSensorModule>();
    }
    else if (module_id == "thruster")
    {
        return std::make_shared<ThrusterModule>();
    }
    else
    {
        throw std::runtime_error("No such module");
    }
}
}