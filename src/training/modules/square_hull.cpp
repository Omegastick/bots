#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>

#include "square_hull.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "training/modules/thruster_module.h"

namespace ai
{
SquareHull::SquareHull() : rectangle{{0.5f, 0.5f, 0.5f, 0.5f},
                                     cl_white,
                                     0.1f,
                                     Transform()}
{
    // Graphics
    rectangle.transform.set_scale({1, 1});

    // Box2D
    b2PolygonShape shape;
    shape.SetAsBox(0.5f, 0.5f);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, 0.5f, 0, this));
    module_links.push_back(ModuleLink(-0.5f, 0, 90, this));
    module_links.push_back(ModuleLink(0, -0.5f, 180, this));
    module_links.push_back(ModuleLink(0.5f, 0, 270, this));
}

void SquareHull::draw(Renderer &renderer, bool /*lightweight*/)
{
    b2Transform world_transform = get_global_transform();
    glm::vec2 screen_position(world_transform.p.x, world_transform.p.y);
    rectangle.transform.set_position(screen_position);
    auto rotation = world_transform.q.GetAngle();
    rectangle.transform.set_rotation(rotation);

    renderer.draw(rectangle);
}

void SquareHull::set_color(const ColorScheme &color_scheme)
{
    rectangle.fill_color = color_scheme.secondary;
    rectangle.stroke_color = color_scheme.primary;
}

nlohmann::json SquareHull::to_json() const
{
    auto json = nlohmann::json::object();
    json["type"] = "square_hull";

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

TEST_CASE("SquareHull converts to correct Json")
{
    SquareHull module;

    auto json = module.to_json();

    SUBCASE("SquareHull Json has correct type")
    {
        CHECK(json["type"] == "square_hull");
    }

    SUBCASE("SquareHull Json has correct number of links")
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
