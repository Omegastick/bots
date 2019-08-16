#include <filesystem>

#include <cpprl/cpprl.h>
#include <nlohmann/json.hpp>

#include "mock_saver.h"

namespace SingularityTrainer
{
nlohmann::json MockSaver::load_json(std::filesystem::path /*path*/)
{
    return json_to_load;
}

void MockSaver::load_policy(std::filesystem::path /*path*/,
                            cpprl::Policy & /*policy*/) {}

void MockSaver::save(cpprl::Policy /*policy*/, std::filesystem::path path)
{
    last_saved_path = path;
}
void MockSaver::save(nlohmann::json json, std::filesystem::path path)
{
    last_saved_path = path;
    last_saved_json = json;
}
}