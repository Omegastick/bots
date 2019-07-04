#include <filesystem>
#include <fstream>

#include <cpprl/cpprl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <torch/torch.h>

#include "saver.h"

namespace SingularityTrainer
{
void create_directory_if_doesnt_exist(std::filesystem::path path)
{
    if (!std::filesystem::exists(path.parent_path()))
    {
        spdlog::info("Creating directory {}", path.string());
        std::filesystem::create_directories(path.parent_path());
    }
}

nlohmann::json Saver::load_json(std::filesystem::path path)
{
    std::ifstream file(path);
    return nlohmann::json::parse(file);
}

void Saver::load_policy(std::filesystem::path path,
                        cpprl::Policy &policy)
{
    torch::load(policy, path.string());
}

void Saver::save(cpprl::Policy policy, std::filesystem::path path)
{
    create_directory_if_doesnt_exist(path);
    torch::save(policy, path.string());
}

void Saver::save(nlohmann::json json, std::filesystem::path path)
{
    create_directory_if_doesnt_exist(path);
    std::ofstream file(path);
    file << json.dump();
}
}