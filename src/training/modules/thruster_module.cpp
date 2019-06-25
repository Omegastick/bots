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
#include "misc/resource_manager.h"
#include "training/actions/activate_action.h"
#include "training/bodies/body.h"
#include "training/modules/imodule.h"
#include "training/modules/thruster_module.h"
#include "training/modules/gun_module.h"
#include "training/rigid_body.h"
#include "misc/random.h"

namespace SingularityTrainer
{
ThrusterModule::ThrusterModule() : active(false), particle_color(cl_white)
{
    // Sprite
    sprite = std::make_unique<Sprite>("thruster_module");
    sprite->set_scale(glm::vec2(1, 0.333));
    sprite->set_origin(sprite->get_center());

    // Box2D fixture
    b2Vec2 vertices[4];
    vertices[0] = b2Vec2(-0.333, -0.167);
    vertices[1] = b2Vec2(-0.5, 0.167);
    vertices[2] = b2Vec2(0.5, 0.167);
    vertices[3] = b2Vec2(0.333, -0.167);
    b2PolygonShape shape;
    shape.Set(vertices, 4);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, 0.167, 0, this));

    actions.push_back(std::make_unique<ActivateAction>(this));
}

void ThrusterModule::activate()
{
    active = true;
    b2Transform global_transform = get_global_transform();
    b2Vec2 velocity = b2Mul(global_transform.q, b2Vec2(0, 50));
    body->get_rigid_body().body->ApplyForce(velocity, global_transform.p, true);
}

RenderData ThrusterModule::get_render_data(bool lightweight)
{
    auto render_data = IModule::get_render_data(lightweight);

    // Spawn particles
    if (active && !lightweight)
    {
        b2Transform global_transform = get_global_transform();
        b2Transform edge_transform = b2Mul(global_transform, b2Transform(b2Vec2(0, -0.3), b2Rot(glm::pi<float>() / 2)));
        std::uniform_real_distribution<float> distribution(0, 1);
        const int particle_count = 20;
        const float step_subdivision = 1.f / particle_count / 10.f;
        glm::vec4 end_color = particle_color;
        end_color.a = 0;
        for (int i = 0; i < particle_count; ++i)
        {
            float random_number = body->get_rng().next_float(distribution) - 0.5;
            b2Rot angle = b2Mul(edge_transform.q, b2Rot(random_number));
            Particle particle{
                glm::vec2(edge_transform.p.x + edge_transform.q.s * random_number, edge_transform.p.y - edge_transform.q.c * random_number),
                -glm::vec2(angle.c * 10, angle.s * 10),
                -i * step_subdivision,
                1,
                0.02,
                particle_color,
                end_color};
            render_data.particles.push_back(particle);
        }
    }

    return render_data;
}

nlohmann::json ThrusterModule::to_json() const
{
    auto json = nlohmann::json::object();
    json["type"] = "thruster";

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
        CHECK(json["type"] == "thruster");
    }

    SUBCASE("ThrusterModule Json has correct number of links")
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