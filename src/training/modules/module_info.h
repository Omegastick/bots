#pragma once

#include <string>

namespace SingularityTrainer
{
struct ModuleInfo
{
    std::string name;
    std::string description;
};

const ModuleInfo &module_info(const std::string &module);
}