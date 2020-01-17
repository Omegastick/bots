#include <algorithm>
#include <limits>
#include <string>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "validate_body.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"
#include "training/modules/gun_module.h"
#include "training/modules/square_hull.h"

namespace ai
{
constexpr float max_size = 10;
constexpr auto too_big_message = "The body is too large";

constexpr unsigned int max_modules = 100;
constexpr auto too_many_modules_message = "There are too many modules on the body";

constexpr auto invalid_module_message = "Module {} is not available";

ValidationResult check_module_json(const nlohmann::json &json,
                                   const std::vector<std::string> &available_modules)
{
    const std::string module_name = json["type"];
    if (std::find(available_modules.begin(),
                  available_modules.end(),
                  module_name) == available_modules.end())
    {
        return ValidationResult{fmt::format(invalid_module_message, module_name), false};
    }

    for (const auto link : json["links"])
    {
        if (link.is_null())
        {
            continue;
        }
        const auto child_result = check_module_json(link["child"], available_modules);
        if (!child_result.ok)
        {
            return child_result;
        }
    }

    return {"", true};
}

ValidationResult validate_body(const Body &body, const std::vector<std::string> &available_modules)
{
    // Reject bodies with too many modules
    if (body.get_modules().size() > max_modules)
    {
        return {too_many_modules_message, false};
    }

    // Reject bodies that are too large
    float x_min = std::numeric_limits<float>::infinity();
    float x_max = -std::numeric_limits<float>::infinity();
    float y_min = std::numeric_limits<float>::infinity();
    float y_max = -std::numeric_limits<float>::infinity();

    for (const auto module : body.get_modules())
    {
        const auto transform = module->get_transform();
        x_min = std::min(x_min, transform.p.x);
        x_max = std::max(x_max, transform.p.x);
        y_min = std::min(y_min, transform.p.y);
        y_max = std::max(y_max, transform.p.y);
    }
    if (x_max - x_min > max_size || y_max - y_min > max_size)
    {
        return {too_big_message, false};
    }

    // Reject bodies with invalid modules
    const auto json = body.to_json();
    const auto module_result = check_module_json(json["base_module"], available_modules);
    if (!module_result.ok)
    {
        return module_result;
    }

    return {"", true};
}

TEST_CASE("validate_body")
{
    Random rng(0);
    MockBulletFactory bullet_factory;
    ModuleFactory module_factory(bullet_factory, rng);
    TestBodyFactory body_factory(module_factory, rng);
    b2World b2_world({0, 0});
    const std::vector<std::string> available_modules = {"base_module",
                                                        "gun_module",
                                                        "laser_sensor_module",
                                                        "thruster_module"};
    auto body = body_factory.make(b2_world, rng);

    SUBCASE("Good bodies should be okay")
    {
        const auto result = validate_body(*body, available_modules);
        DOCTEST_CHECK(result.ok);
    }

    SUBCASE("Bodies using an invalid module should be rejected")
    {
        const auto hull_module = module_factory.make("square_hull");
        auto body_modules = body->get_modules();
        for (auto &module : body_modules)
        {
            if (module->to_json()["type"] == "gun_module")
            {
                for (auto &module_link : module->get_module_links())
                {
                    if (!module_link.linked)
                    {
                        module_link.link(hull_module->get_module_links()[0]);
                        break;
                    }
                }
                break;
            }
        }
        body->add_module(hull_module);

        const auto result = validate_body(*body, available_modules);
        DOCTEST_CHECK(!result.ok);
    }

    SUBCASE("Bodies that are too large in the X dimension should be rejected")
    {
        auto *module = body->get_modules()[1].get();
        auto *module_link = &module->get_module_links()[0];
        for (int i = 0; i < 10; ++i)
        {
            auto new_module = module_factory.make("gun_module");
            module_link->link(new_module->get_module_links()[2]);
            body->add_module(new_module);
            module = new_module.get();
            module_link = &new_module->get_module_links()[0];
        }

        const auto result = validate_body(*body, available_modules);
        DOCTEST_CHECK(!result.ok);
    }

    SUBCASE("Bodies with too many modules should be rejected")
    {
        for (int i = 0; i < 100; ++i)
        {
            auto new_module = module_factory.make("gun_module");
            body->add_module(new_module);
        }

        const auto result = validate_body(*body, available_modules);
        DOCTEST_CHECK(!result.ok);
    }
}
}