#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "graphics/colors.h"
#include "resource_manager.h"
#include "training/agents/agent.h"
#include "training/modules/base_module.h"
#include "training/modules/imodule.h"
#include "training/modules/module_link.h"
#include "training/modules/gun_module.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
BaseModule::BaseModule()
{
    // Sprite
    sprite = std::make_unique<Sprite>("base_module");
    sprite->set_scale(glm::vec2(1, 1));
    sprite->set_origin(sprite->get_center());
    sprite->set_color(cl_white);

    // Box2D
    b2PolygonShape shape;
    shape.SetAsBox(0.5, 0.5);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, 0.5, 0, this));
    module_links.push_back(ModuleLink(0.5, 0, 90, this));
    module_links.push_back(ModuleLink(0, -0.5, 180, this));
    module_links.push_back(ModuleLink(-0.5, 0, 270, this));

    root = this;
}

BaseModule::~BaseModule() {}

std::vector<float> BaseModule::get_sensor_reading()
{
    b2Vec2 linear_velocity = agent->get_rigid_body()->body->GetLinearVelocity();
    float angular_velocity = agent->get_rigid_body()->body->GetAngularVelocity();
    return {linear_velocity.x, linear_velocity.y, angular_velocity};
}

nlohmann::json BaseModule::to_json() const
{
    auto json = nlohmann::json::object();
    json["type"] = "base";

    json["links"] = nlohmann::json::array();
    for (const auto &link : module_links)
    {
        if (link.linked && link.is_parent)
        {
            int child_link = 0;
            while (link.linked_module->get_module_links()[child_link].linked_module != this)
            {
                child_link++;
            }
            json["links"].push_back(nlohmann::json::object());
            json["links"].back()["child_link"] = child_link;
            json["links"].back()["child"] = link.linked_module->to_json();
        }
        else
        {
            json["links"].push_back(nullptr);
        }
    }

    return json;
}

TEST_CASE("BaseModule converts to correct Json")
{
    BaseModule module;

    auto json = module.to_json();

    SUBCASE("BaseModule Json has correct type")
    {
        CHECK(json["type"] == "base");
    }

    SUBCASE("BaseModule Json has correct number of links")
    {
        CHECK(json["links"].size() == 4);
    }

    SUBCASE("Nested modules are represented correctly in Json")
    {
        // Attach gun module
        GunModule gun_module;
        module.get_module_links()[0].link(&gun_module.get_module_links()[0]);

        // Update json
        json = module.to_json();

        SUBCASE("Submodule Json has correct type")
        {
            CHECK(json["links"][0]["child"]["type"] == "gun");
        }

        SUBCASE("Link's child link number is correct")
        {
            CHECK(json["links"][0]["child_link"] == 0);
        }

        SUBCASE("Submodule link to parent is null in Json")
        {
            CHECK(json["links"][0]["child"]["links"][0] == nullptr);
        }
    }
}
}