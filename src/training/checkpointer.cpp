#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

#include <doctest.h>
#include <cpprl/cpprl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "checkpointer.h"
#include "third_party/date.h"
#include "misc/random.h"
#include "training/mock_saver.h"

namespace fs = std::filesystem;

namespace ai
{
static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";
const std::string schema_version = "v1alpha1";

Checkpointer::Checkpointer(std::string checkpoint_directory,
                           Random &random,
                           ISaver &saver)
    : checkpoint_directory(fs::current_path() / checkpoint_directory),
      random(random),
      saver(saver)
{
    if (!fs::exists(this->checkpoint_directory))
    {
        spdlog::info("Creating directory {}", this->checkpoint_directory.string());
        fs::create_directories(this->checkpoint_directory);
    }
}

std::vector<fs::path> Checkpointer::enumerate_checkpoints()
{
    std::vector<fs::path> paths;
    for (const auto &file : fs::directory_iterator(checkpoint_directory))
    {
        if (file.path().extension() == ".meta")
        {
            paths.push_back(file.path());
        }
    }
    return paths;
}

Checkpoint Checkpointer::load(fs::path path)
{
    auto data = load_data(path);

    auto nn_base = std::make_shared<cpprl::MlpBase>(data.body_spec["num_observations"],
                                                    data.recurrent);
    cpprl::Policy policy(cpprl::ActionSpace{"MultiBinary",
                                            {data.body_spec["num_actions"]}},
                         nn_base,
                         true);
    auto policy_path = path.replace_extension(".pth");
    saver.load_policy(policy_path, policy);

    return {data, policy};
}

CheckpointData Checkpointer::load_data(fs::path path)
{
    auto json = saver.load_json(path);
    std::chrono::system_clock::time_point time_stamp;
    std::istringstream string_stream(static_cast<std::string>(json["timestamp"]));
    string_stream >> date::parse("%F-%H-%M-%S", time_stamp);
    return {json["body_spec"],
            json["data"],
            json["previous_checkpoint"].get<std::string>(),
            json["recurrent"],
            time_stamp};
}
fs::path Checkpointer::save(cpprl::Policy &policy,
                            nlohmann::json &body_spec,
                            std::map<std::string, double> data,
                            fs::path previous_checkpoint,
                            fs::path directory)
{
    std::string file_id;
    for (int i = 0; i < 10; ++i)
    {
        file_id += alphanum[random.next_int(0, sizeof(alphanum) - 1)];
    }

    fs::path save_path;
    if (directory.empty())
    {
        save_path = checkpoint_directory / file_id;
    }
    else
    {
        save_path = directory / file_id;
        if (!fs::exists(directory))
        {
            spdlog::info("Creating directory {}", directory.string());
            fs::create_directories(directory);
        }
    }

    auto model_path = save_path.replace_extension(".pth");
    spdlog::debug("Saving model to {}", model_path.string());
    saver.save(policy, model_path);

    nlohmann::json json;
    json["schema"] = schema_version;
    json["body_spec"] = body_spec;
    json["data"] = data;
    json["previous_checkpoint"] = previous_checkpoint.string();
    json["recurrent"] = policy->is_recurrent();
    json["timestamp"] = date::format("%F-%H-%M-%S", std::chrono::system_clock::now());
    auto meta_path = save_path.replace_extension(".meta");
    saver.save(json, meta_path);

    return meta_path;
}

TEST_CASE("Checkpointer")
{
    MockSaver saver;
    Random random(0);
    Checkpointer checkpointer("/tmp/checkpoints/", random, saver);

    SUBCASE("save()")
    {
        auto nn_base = std::make_shared<cpprl::MlpBase>(5, false);
        cpprl::Policy policy(cpprl::ActionSpace{"MultiBinary", {12}}, nn_base);
        // clang-format off
        nlohmann::json body_spec = {
            {"base_module", {
                {"links", {nullptr, nullptr, nullptr, nullptr}},
                    {"type", "base_module"}}
                },
            {"name", "qweqwe"},
            {"num_observations", 5},
            {"num_actions", 12},
            {"schema", "v1alpha2"}
        };
        // clang-format on
        checkpointer.save(policy, body_spec, {{"asd", 123}, {"sdf", 5.43}}, "/asd/sdf.meta");

        SUBCASE("Saves files to the correct paths")
        {
            auto path = saver.last_saved_path.string();
            DOCTEST_INFO(path);
            DOCTEST_CHECK_UNARY(path.substr(0, 17) == "/tmp/checkpoints/" ||
                                path.substr(0, 19) == "C:/tmp/checkpoints/");
        }

        SUBCASE("Saves correct Json")
        {
            auto json = saver.last_saved_json;
            DOCTEST_CHECK(json["schema"] == schema_version);
            DOCTEST_CHECK(json["body_spec"] == body_spec);
            DOCTEST_CHECK(json["data"]["asd"] == doctest::Approx(123));
            DOCTEST_CHECK(json["data"]["sdf"] == doctest::Approx(5.43));
            DOCTEST_CHECK(json["previous_checkpoint"] == "/asd/sdf.meta");
            DOCTEST_CHECK(json["recurrent"] == false);
        }

        SUBCASE("Saves accurate timestamps")
        {
            auto json = saver.last_saved_json;
            std::chrono::system_clock::time_point time_stamp;
            std::istringstream string_stream(static_cast<std::string>(json["timestamp"]));
            string_stream >> date::parse("%F-%H-%M-%S", time_stamp);
            DOCTEST_CHECK(std::chrono::system_clock::now() - time_stamp < std::chrono::seconds(2));
        }
    }

    SUBCASE("load_data() loads correct data")
    {
        // clang-format off
            saver.json_to_load = {
                {"schema", schema_version},
                {"body_spec", {
                    {"base_module", {
                        {"links", {nullptr, nullptr, nullptr, nullptr}},
                        {"type", "base_module"}}
                    },
                    {"name", "qweqwe"},
                    {"num_observations", 5},
                    {"num_actions", 12},
                    {"schema", "v1alpha2"}}
                },
                {"data", {
                    {"asd", 123},
                    {"sdf", 5.43}}
                },
                {"previous_checkpoint", "/asd/sdf.meta"},
                {"recurrent", false},
                {"timestamp", date::format("%F-%H-%M-%S", std::chrono::system_clock::now())}};
        // clang-format on
        auto data = checkpointer.load_data("/asd");

        DOCTEST_CHECK(data.body_spec["schema"] == "v1alpha2");
        DOCTEST_CHECK(data.body_spec["base_module"]["type"] == "base_module");
        DOCTEST_CHECK(data.data["asd"] == doctest::Approx(123));
        DOCTEST_CHECK(data.data["sdf"] == doctest::Approx(5.43));
        DOCTEST_CHECK(data.previous_checkpoint == "/asd/sdf.meta");
        DOCTEST_CHECK(data.recurrent == false);
    }
}
}