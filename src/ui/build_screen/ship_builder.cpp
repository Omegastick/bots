#include <memory>

#include <doctest/doctest.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "ui/build_screen/ship_builder.h"
#include "training/agents/agent.h"
#include "training/modules/module_link.h"

namespace SingularityTrainer
{
ShipBuilder::ShipBuilder() {}
IModule *ShipBuilder::get_module_at_point(glm::vec2 /*point*/) { return nullptr; }
ModuleLinkAndDistance ShipBuilder::get_nearest_module_link(glm::vec2 /*point*/) { return {}; }
void ShipBuilder::update() {}

TEST_CASE("ShipBuilder")
{
    ShipBuilder ship_builder;

    SUBCASE("Starts with only a single base module")
    {
        auto agent = ship_builder.get_agent();
        auto modules = agent->get_modules();

        CHECK(modules.size() == 1);

        auto transform = modules[0]->get_global_transform();
        CHECK(transform.p.Length() == 0);
        CHECK(transform.q.GetAngle() == 0);

        auto json = modules[0]->to_json();
        CHECK(json["type"] == "base");
    }

    SUBCASE("get_module_at_point()")
    {
        SUBCASE("Correctly selects the base module at 0, 0")
        {
            auto selected_module = ship_builder.get_module_at_point({0, 0});
            auto base_module = ship_builder.get_agent()->get_modules()[0].get();

            CHECK(selected_module == base_module);
        }

        SUBCASE("Returns a nullptr if no module at exists at the point")
        {
            auto selected_module = ship_builder.get_module_at_point({1000, 1000});

            CHECK(selected_module == nullptr);
        }
    }

    SUBCASE("get_nearest_module_link()")
    {
        SUBCASE("Selects the correct link under normal use")
        {
            auto selected_link = ship_builder.get_nearest_module_link({960, 0}).module_link;
            auto expected_link = &ship_builder.get_agent()->get_modules()[0]->get_module_links()[0];

            CHECK(selected_link == expected_link);
        }
    }
}
}