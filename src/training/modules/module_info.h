#pragma once

#include <string>

namespace ai
{
struct ModuleInfo
{
    std::string name;
    std::string description;
};

const ModuleInfo &module_info(const std::string &module);
}