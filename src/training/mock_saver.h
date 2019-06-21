#pragma once
#include <filesystem>

#include <cpprl/cpprl.h>
#include <nlohmann/json.hpp>

#include "isaver.h"

namespace SingularityTrainer
{
class MockSaver : public ISaver
{
  public:
    std::filesystem::path last_saved_path;
    nlohmann::json last_saved_json;
    nlohmann::json json_to_load;

    virtual nlohmann::json load_json(std::filesystem::path path);
    virtual cpprl::Policy load_policy(std::filesystem::path path, cpprl::Policy policy);
    virtual void save(cpprl::Policy policy, std::filesystem::path path);
    virtual void save(nlohmann::json json, std::filesystem::path path);
};
}