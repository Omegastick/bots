#pragma once
#include <filesystem>

#include <cpprl/model/policy.h>
#include <nlohmann/json_fwd.hpp>

namespace SingularityTrainer
{
class ISaver
{
  public:
    virtual ~ISaver() = 0;

    virtual nlohmann::json load_json(std::filesystem::path path) = 0;
    virtual cpprl::Policy load_policy(std::filesystem::path path, cpprl::Policy policy) = 0;
    virtual void save(cpprl::Policy policy, std::filesystem::path path) = 0;
    virtual void save(nlohmann::json json, std::filesystem::path path) = 0;
};

inline ISaver::~ISaver() {}
}