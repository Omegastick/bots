#pragma once
#include <filesystem>

#include <cpprl/model/policy.h>
#include <nlohmann/json_fwd.hpp>

#include "isaver.h"

namespace ai
{
class Saver : public ISaver
{
  public:
    virtual nlohmann::json load_json(std::filesystem::path path);
    virtual void load_policy(std::filesystem::path path, cpprl::Policy &policy);
    virtual void save(cpprl::Policy policy, std::filesystem::path path);
    virtual void save(nlohmann::json json, std::filesystem::path path);
};
}