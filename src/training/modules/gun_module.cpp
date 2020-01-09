#include <iostream>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>

#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "training/actions/activate_action.h"
#include "training/bodies/body.h"
#include "training/entities/bullet.h"
#include "training/environments/ienvironment.h"
#include "training/modules/gun_module.h"
#include "training/modules/imodule.h"
#include "training/rigid_body.h"

namespace ai
{
GunModule::GunModule(IBulletFactory &bullet_factory, Random &rng)
    : barrel_rectangle{{0.5f, 0.5f, 0.5f, 0.5f},
                       cl_white,
                       0.1f,
                       Transform()},
      body_rectangle{{0.5f, 0.5f, 0.5f, 0.5f},
                     cl_white,
                     0.1f,
                     Transform()},
      bullet_factory(bullet_factory),
      cooldown(3),
      rng(rng),
      steps_since_last_shot(0)
{
    // Graphics
    body_rectangle.transform.set_scale({1.f, 0.666f});
    body_rectangle.transform.set_origin({0.f, 0.167f});
    barrel_rectangle.transform.set_scale({0.333f, 0.333});
    barrel_rectangle.transform.set_origin({0.f, -0.333f});

    // Box2D fixture
    b2PolygonShape body_shape;
    body_shape.SetAsBox(0.5f, 0.333f, b2Vec2(0, 0.167f), 0);
    shapes.push_back(body_shape);
    b2PolygonShape barrel_shape;
    barrel_shape.SetAsBox(0.167f, 0.167f, b2Vec2(0, -0.167f), 0);
    shapes.push_back(barrel_shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(-0.5f, -0.167f, 90, this));
    module_links.push_back(ModuleLink(0, -0.5f, 180, this));
    module_links.push_back(ModuleLink(0.5f, -0.167f, 270, this));

    actions.push_back(std::make_unique<ActivateAction>(this));
}

void GunModule::activate()
{
    if (steps_since_last_shot > cooldown)
    {
        steps_since_last_shot = 0;
        b2Transform global_transform = get_global_transform();
        b2Vec2 velocity = b2Mul(global_transform.q, b2Vec2(0, 100));
        b2Vec2 offset = b2Mul(global_transform.q, b2Vec2(0, 0.7f));
        body->get_environment()->add_entity(
            bullet_factory.make(global_transform.p + offset,
                                velocity,
                                *body->get_rigid_body().body->GetWorld(),
                                body,
                                rng.next_int(0, INT_MAX),
                                *body->get_environment()));
    }
}

void GunModule::draw(Renderer &renderer, bool /*lightweight*/)
{
    b2Transform world_transform = get_global_transform();
    glm::vec2 screen_position(world_transform.p.x, world_transform.p.y);
    body_rectangle.transform.set_position(screen_position);
    barrel_rectangle.transform.set_position(screen_position);
    auto rotation = world_transform.q.GetAngle();
    body_rectangle.transform.set_rotation(rotation);
    barrel_rectangle.transform.set_rotation(rotation);

    renderer.draw(body_rectangle);
    renderer.draw(barrel_rectangle);
}

void GunModule::set_color(const ColorScheme &color_scheme)
{
    body_rectangle.fill_color = color_scheme.secondary;
    barrel_rectangle.fill_color = color_scheme.secondary;
    body_rectangle.stroke_color = color_scheme.primary;
    barrel_rectangle.stroke_color = color_scheme.primary;
}

nlohmann::json GunModule::to_json() const
{
    auto json = nlohmann::json::object();
    json["type"] = "gun_module";

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
}

TEST_CASE("GunModule converts to correct Json")
{
    Random rng(0);
    MockBulletFactory bullet_factory;
    GunModule module(bullet_factory, rng);

    auto json = module.to_json();

    SUBCASE("GunModule Json has correct type")
    {
        DOCTEST_CHECK(json["type"] == "gun_module");
    }

    SUBCASE("GunModule Json has correct number of links")
    {
        DOCTEST_CHECK(json["links"].size() == 3);
    }

    SUBCASE("Nested modules are represented correctly in Json")
    {
        // Attach gun module
        GunModule gun_module(bullet_factory, rng);
        module.get_module_links()[0].link(gun_module.get_module_links()[0]);

        // Update json
        json = module.to_json();

        SUBCASE("Submodule Json has correct type")
        {
            DOCTEST_CHECK(json["links"][0]["child"]["type"] == "gun_module");
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