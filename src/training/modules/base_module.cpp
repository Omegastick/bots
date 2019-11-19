#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <glm/vec2.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "misc/resource_manager.h"
#include "misc/transform.h"
#include "training/bodies/body.h"
#include "training/environments/ienvironment.h"
#include "training/modules/base_module.h"
#include "training/modules/imodule.h"
#include "training/modules/module_link.h"
#include "training/modules/thruster_module.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
BaseModule::BaseModule() : rectangle{{0.5f, 0.5f, 0.5f, 0.5f},
                                     cl_white,
                                     0.1f,
                                     Transform()},
                           circle{0.2f,
                                  {0, 0, 0, 0},
                                  cl_white,
                                  0.1f,
                                  Transform()}
{
    // Graphics
    rectangle.transform.set_scale({1, 1});

    // Box2D
    b2PolygonShape shape;
    shape.SetAsBox(0.5, 0.5);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, 0.5f, 0, this));
    module_links.push_back(ModuleLink(-0.5f, 0, 90, this));
    module_links.push_back(ModuleLink(0, -0.5f, 180, this));
    module_links.push_back(ModuleLink(0.5f, 0, 270, this));

    root = this;
}

void BaseModule::draw(Renderer &renderer, bool /*lightweight*/)
{
    b2Transform world_transform = get_global_transform();
    glm::vec2 screen_position(world_transform.p.x, world_transform.p.y);
    rectangle.transform.set_position(screen_position);
    circle.transform.set_position(screen_position);
    auto rotation = world_transform.q.GetAngle();
    rectangle.transform.set_rotation(rotation);

    renderer.draw(rectangle);
    renderer.draw(circle);
}

std::vector<float> BaseModule::get_sensor_reading() const
{
    b2Vec2 linear_velocity = body->get_rigid_body().body->GetLinearVelocity();
    linear_velocity = b2Mul(b2Rot(body->get_rigid_body().body->GetAngle()), linear_velocity);
    float angular_velocity = body->get_rigid_body().body->GetAngularVelocity();
    return {linear_velocity.x, linear_velocity.y, angular_velocity};
}

void BaseModule::set_color(glm::vec4 color)
{
    auto fill = color;
    fill.a = 0.2f;

    circle.stroke_color = color;
    rectangle.fill_color = fill;
    rectangle.stroke_color = color;
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
        // Attach thruster module
        ThrusterModule thruster_module;
        module.get_module_links()[0].link(thruster_module.get_module_links()[0]);

        // Update json
        json = module.to_json();

        SUBCASE("Submodule Json has correct type")
        {
            CHECK(json["links"][0]["child"]["type"] == "thruster");
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
