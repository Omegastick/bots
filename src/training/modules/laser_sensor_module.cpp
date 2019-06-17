#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "graphics/colors.h"
#include "misc/resource_manager.h"
#include "training/agents/agent.h"
#include "training/modules/base_module.h"
#include "training/modules/imodule.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/module_link.h"
#include "training/modules/gun_module.h"
#include "training/rigid_body.h"
#include "misc/utilities.h"

namespace SingularityTrainer
{
LaserSensorModule::LaserSensorModule(int laser_count, float fov, float laser_length)
    : laser_count(laser_count),
      fov(fov),
      laser_length(laser_length)
{
    // Sprite
    sprite = std::make_unique<Sprite>("laser_sensor_module");
    sprite->set_scale(glm::vec2(1, 0.5));
    sprite->set_origin(sprite->get_center());
    sprite->set_color(cl_white);

    // Box2D
    b2PolygonShape shape;
    shape.SetAsBox(0.5, 0.25);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, -0.25, 180, this));
}

std::vector<float> LaserSensorModule::get_sensor_reading()
{
    std::vector<float> sensor_reading;
    sensor_reading.resize(laser_count);

    b2Transform global_transform = get_global_transform();
    float segment_width = fov / (laser_count - 1);

    for (int i = 0; i < laser_count; ++i)
    {
        ClosestRaycastCallback raycast_callback;

        b2Rot angle(glm::radians((segment_width * i) - (fov / 2)));
        b2Vec2 laser = b2Mul(angle, b2Vec2(0, laser_length));

        agent->get_rigid_body().body->GetWorld()->RayCast(&raycast_callback, global_transform.p, b2Mul(global_transform, laser));
        if (raycast_callback.distance == -1)
        {
            sensor_reading[i] = 1;
        }
        else
        {
            sensor_reading[i] = raycast_callback.distance;
        }
    }

    last_reading = sensor_reading;
    return sensor_reading;
}

RenderData LaserSensorModule::get_render_data(bool lightweight)
{
    auto render_data = IModule::get_render_data(lightweight);

    if (!lightweight && last_reading.size() > 0)
    {
        b2Transform global_transform = get_global_transform();
        float segment_width = fov / (laser_count - 1);

        Line line;
        glm::vec4 start_color = cl_white;
        start_color.a = 0;

        for (int i = 0; i < laser_count; ++i)
        {
            if (last_reading[i] < 1)
            {
                Line line;
                b2Rot angle(glm::radians((segment_width * i) - (fov / 2)));
                b2Vec2 laser = b2Mul(angle, b2Vec2(0, last_reading[i] * laser_length));
                b2Vec2 laser_start = b2Mul(angle, b2Vec2(0, 0.35));
                b2Vec2 transformed_end = b2Mul(global_transform, laser);
                b2Vec2 transformed_start = b2Mul(global_transform, laser_start);
                glm::vec4 end_color = start_color;
                end_color.a = last_reading[i];
                line.points.push_back({transformed_start.x, transformed_start.y});
                line.colors.push_back(start_color);
                line.widths.push_back(0.01);
                line.points.push_back({transformed_end.x, transformed_end.y});
                line.colors.push_back(end_color);
                line.widths.push_back(0.01);
                render_data.lines.push_back(line);
            }
        }
    }

    return render_data;
}

nlohmann::json LaserSensorModule::to_json() const
{
    auto json = nlohmann::json::object();
    json["type"] = "laser_sensor";

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

ClosestRaycastCallback::ClosestRaycastCallback() : distance(-1) {}
ClosestRaycastCallback::~ClosestRaycastCallback() {}

float32 ClosestRaycastCallback::ReportFixture(b2Fixture *fixture, const b2Vec2 & /*point*/, const b2Vec2 & /*normal*/, float32 fraction)
{
    auto fixture_type = static_cast<RigidBody *>(fixture->GetBody()->GetUserData())->parent_type;
    if (fixture_type == RigidBody::ParentTypes::Bullet || fixture_type == RigidBody::ParentTypes::Hill)
    {
        return 1;
    }
    distance = fraction;
    return fraction;
}

TEST_CASE("LaserSensorModule converts to correct Json")
{
    LaserSensorModule module;

    auto json = module.to_json();

    SUBCASE("LaserSensorModule Json has correct type")
    {
        CHECK(json["type"] == "laser_sensor");
    }

    SUBCASE("LaserSensorModule Json has correct number of links")
    {
        CHECK(json["links"].size() == 1);
    }

    SUBCASE("Nested modules are represented correctly in Json")
    {
        // Attach gun module
        GunModule gun_module;
        module.get_module_links()[0].link(gun_module.get_module_links()[0]);

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
