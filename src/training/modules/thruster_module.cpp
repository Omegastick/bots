#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <nlohmann/json.hpp>

#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "misc/resource_manager.h"
#include "training/actions/activate_action.h"
#include "training/bodies/body.h"
#include "training/environments/ienvironment.h"
#include "training/events/effect_triggered.h"
#include "training/modules/imodule.h"
#include "training/modules/thruster_module.h"
#include "training/rigid_body.h"
#include "misc/random.h"

namespace ai
{
ThrusterModule::ThrusterModule() : active(false),
                                   trapezoid{0.666f,
                                             1.f,
                                             {0.5f, 0.5f, 0.5f, 0.5f},
                                             cl_white,
                                             0.1f,
                                             Transform()}
{
    // Graphics
    trapezoid.transform.set_scale({1.f, 0.25f});

    // Box2D fixture
    b2Vec2 vertices[4];
    vertices[0] = b2Vec2(-0.333f, -0.125f);
    vertices[1] = b2Vec2(-0.5f, 0.125f);
    vertices[2] = b2Vec2(0.5f, 0.125f);
    vertices[3] = b2Vec2(0.333f, -0.125f);
    b2PolygonShape shape;
    shape.Set(vertices, 4);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, 0.125f, 0, this));

    actions.push_back(std::make_unique<ActivateAction>(this));
}

void ThrusterModule::activate()
{
    active = true;
    b2Transform global_transform = get_global_transform();
    b2Vec2 velocity = b2Mul(global_transform.q, b2Vec2(0, 50));
    body->get_rigid_body().body->ApplyForce(velocity, global_transform.p, true);
}

void ThrusterModule::draw(Renderer &renderer, bool /*lightweight*/)
{
    b2Transform world_transform = get_global_transform();
    glm::vec2 screen_position(world_transform.p.x, world_transform.p.y);
    trapezoid.transform.set_position(screen_position);
    auto rotation = world_transform.q.GetAngle();
    trapezoid.transform.set_rotation(rotation);

    renderer.draw(trapezoid);
}

void ThrusterModule::set_color(const ColorScheme &color_scheme)
{
    trapezoid.fill_color = color_scheme.secondary;
    trapezoid.stroke_color = color_scheme.primary;
}

nlohmann::json ThrusterModule::to_json() const
{
    auto json = nlohmann::json::object();
    json["type"] = "thruster_module";

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

void ThrusterModule::sub_update()
{
    b2Transform global_transform = get_global_transform();
    if (active)
    {
        body->get_environment()->add_event(std::make_unique<EffectTriggered>(
            EffectTypes::ThrusterParticles,
            body->get_environment()->get_elapsed_time(),
            Transform{global_transform.p.x, global_transform.p.y, global_transform.q.GetAngle()}));
    }
}

void ThrusterModule::update()
{
    active = false;
}

TEST_CASE("ThrusterModule converts to correct Json")
{
    ThrusterModule module;

    auto json = module.to_json();

    SUBCASE("ThrusterModule Json has correct type")
    {
        DOCTEST_CHECK(json["type"] == "thruster_module");
    }

    SUBCASE("ThrusterModule Json has correct number of links")
    {
        DOCTEST_CHECK(json["links"].size() == 1);
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
            DOCTEST_CHECK(json["links"][0]["child"]["type"] == "thruster_module");
        }

        SUBCASE("Link's child link number is correct")
        {
            DOCTEST_CHECK(json["links"][0]["child_link"] == 0);
        }

        SUBCASE("Submodule link to parent is null in Json")
        {
            DOCTEST_CHECK(json["links"][0]["child"]["links"][0] == nullptr);
        }
    }
}
}