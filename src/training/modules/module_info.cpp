#include <string>
#include <unordered_map>

#include "module_info.h"

namespace SingularityTrainer
{
const std::unordered_map<std::string, const ModuleInfo> module_info_map{
    {"base_module",
     {"Base Module",
      "The core of any body. Also provides the AI with information about its current velocity."}},
    {"gun_module",
     {"Gun Module",
      "Shoots bullets. Can be used by activated by the AI."}},
    {"laser_sensor_module",
     {"Laser Sensor Module",
      "Uses an array of lasers to 'see' things."}},
    {"square_hull",
     {"Square Hull",
      "An inert, square hull piece. Doesn't do anything in particular."}},
    {"thruster_module",
     {"Thruster Module",
      "Pushes the body forwards. Can be turned off and on by the AI."}},
};

const ModuleInfo &module_info(const std::string &module)
{
    return module_info_map.at(module);
}
}