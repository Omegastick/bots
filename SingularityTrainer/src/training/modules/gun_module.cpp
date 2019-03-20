#include <iostream>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>

#include "graphics/sprite.h"
#include "resource_manager.h"
#include "training/actions/activate_action.h"
#include "training/agents/agent.h"
#include "training/entities/bullet.h"
#include "training/modules/gun_module.h"
#include "training/modules/imodule.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
GunModule::GunModule() : cooldown(3), steps_since_last_shot(0)
{
    // Sprite
    sprite = std::make_unique<Sprite>("gun_module");
    sprite->set_scale(glm::vec2(1, 1));
    sprite->set_origin(sprite->get_center());

    // Box2D fixture
    b2PolygonShape body_shape;
    body_shape.SetAsBox(0.5, 0.333, b2Vec2(0, 0.167), 0);
    shapes.push_back(body_shape);
    b2PolygonShape barrel_shape;
    barrel_shape.SetAsBox(0.167, 0.333, b2Vec2(0, -0.167), 0);
    shapes.push_back(barrel_shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0.5, -0.167, 90, this));
    module_links.push_back(ModuleLink(0, -0.5, 180, this));
    module_links.push_back(ModuleLink(-0.5, -0.167, 270, this));

    actions.push_back(std::make_unique<ActivateAction>(this));
}

void GunModule::activate()
{
    if (steps_since_last_shot > cooldown)
    {
        steps_since_last_shot = 0;
        b2Transform global_transform = get_global_transform();
        b2Vec2 velocity = b2Mul(global_transform.q, b2Vec2(0, 100));
        b2Vec2 offset = b2Mul(global_transform.q, b2Vec2(0, 0.7));
        bullets.push_back(std::make_unique<Bullet>(global_transform.p + offset, velocity, *agent->get_rigid_body()->body->GetWorld(), agent));
    }
}

RenderData GunModule::get_render_data(bool lightweight)
{
    auto render_data = IModule::get_render_data(lightweight);
    for (const auto &bullet : bullets)
    {
        render_data.append(bullet->get_render_data(lightweight));
    }
    return render_data;
}

nlohmann::json GunModule::to_json() const
{
    auto json = nlohmann::json::object();
    json["type"] = "gun";

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

void GunModule::update()
{
    steps_since_last_shot++;

    for (const auto &bullet : bullets)
    {
        bullet->update();
    }

    for (unsigned int i = 0; i < bullets.size(); ++i)
    {
        if (bullets[i]->destroyed)
        {
            b2Body *body = bullets[i]->rigid_body->body;
            body->GetWorld()->DestroyBody(body);
            bullets.erase(bullets.begin() + i);
        }
    }
}

TEST_CASE("GunModule converts to correct Json")
{
    GunModule module;

    auto json = module.to_json();

    SUBCASE("GunModule Json has correct type")
    {
        CHECK(json["type"] == "gun");
    }

    SUBCASE("GunModule Json has correct number of links")
    {
        CHECK(json["links"].size() == 3);
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