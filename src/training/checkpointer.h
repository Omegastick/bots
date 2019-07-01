#pragma once
#include <chrono>
#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

#include <nlohmann/json.hpp>

#include "third_party/di.hpp"
#include "training/isaver.h"

namespace cpprl
{
class Policy;
}

namespace SingularityTrainer
{
class Random;

struct CheckpointData
{
    nlohmann::json body_spec;
    std::map<std::string, double> data;
    std::filesystem::path previous_checkpoint;
    bool recurrent;
    std::chrono::time_point<std::chrono::system_clock> save_time;
};

struct Checkpoint
{
    CheckpointData data;
    cpprl::Policy policy;
};

auto CheckpointDirectory = [] {};

class Checkpointer
{
  private:
    std::filesystem::path checkpoint_directory;
    Random &random;
    ISaver &saver;

  public:
    BOOST_DI_INJECT(Checkpointer,
                    (named = CheckpointDirectory) std::string checkpoint_directory,
                    Random &random,
                    ISaver &saver);

    std::vector<std::filesystem::path> enumerate_checkpoints();
    Checkpoint load(std::filesystem::path path);
    CheckpointData load_data(std::filesystem::path path);
    std::filesystem::path save(cpprl::Policy &policy,
                               nlohmann::json &body_spec,
                               std::map<std::string, double> data,
                               std::filesystem::path previous_checkpoint,
                               std::filesystem::path directory = {});
};
}