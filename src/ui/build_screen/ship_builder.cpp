#include <memory>

#include <Box2D/Box2D.h>
#include <doctest/doctest.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "ui/build_screen/ship_builder.h"
#include "graphics/render_data.h"
#include "io.h"
#include "random.h"
#include "training/agents/agent.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/module_link.h"
#include "training/rigid_body.h"
#include "utilities.h"

namespace SingularityTrainer
{
const double max_link_distance = 1;

bool GetAllQueryCallback::ReportFixture(b2Fixture *fixture)
{
    fixtures.push_back(fixture);
    return true;
}

ShipBuilder::ShipBuilder(b2World &b2_world, Random &rng, IO &io)
    : agent(Agent(b2_world, &rng)),
      io(&io),
      b2_world(&b2_world),
      projection(glm::ortho(-9.6f, 9.6f, -5.4f, 5.4f))
{
    auto base_module = std::make_shared<BaseModule>();
    agent.add_module(base_module);
    agent.update_body();
}

void ShipBuilder::delete_module(std::shared_ptr<IModule> module)
{
    if (module == agent.get_modules()[0])
    {
        spdlog::error("Can't delete base module");
        return;
    }
    agent.unlink_module(module);
    agent.update_body();
}

std::shared_ptr<IModule> ShipBuilder::get_module_at_screen_position(glm::vec2 point)
{
    point = screen_to_world_space(point, static_cast<glm::vec2>(io->get_resolution()), projection);

    GetAllQueryCallback query_callback;
    b2_world->QueryAABB(&query_callback, {{point.x, point.y},
                                          {point.x, point.y}});

    auto collisions = query_callback.get_collisions();
    if (collisions.size() == 0)
    {
        return nullptr;
    }

    auto fixture = collisions[0];
    auto rigid_body = static_cast<RigidBody *>(fixture->GetBody()->GetUserData());
    if (rigid_body->parent_type != RigidBody::ParentTypes::Agent)
    {
        return nullptr;
    }

    return *static_cast<std::shared_ptr<IModule> *>(fixture->GetUserData());
}

NearestModuleLinkResult ShipBuilder::get_nearest_module_link_to_world_position(glm::vec2 point)
{
    b2Vec2 b2_point(point.x, point.y);

    ModuleLink *nearest_link = nullptr;
    double nearest_distance = INFINITY;

    for (const auto &module : agent.get_modules())
    {
        for (auto &module_link : module->get_module_links())
        {
            if (module_link.linked)
            {
                continue;
            }
            double distance = (module_link.get_global_transform().p - b2_point).LengthSquared();
            if (distance < nearest_distance)
            {
                nearest_link = &module_link;
                nearest_distance = distance;
            }
        }
    }

    return {nearest_link, nullptr, nearest_distance};
}

NearestModuleLinkResult ShipBuilder::get_nearest_module_link_to_module(IModule &module)
{
    double nearest_distance = INFINITY;
    ModuleLink *nearest_link = nullptr;
    ModuleLink *origin_link = nullptr;

    for (auto &module_link : module.get_module_links())
    {
        auto world_position = module_link.get_global_transform().p;
        auto link_and_distance = get_nearest_module_link_to_world_position({world_position.x, world_position.y});
        if (link_and_distance.nearest_link != nullptr && link_and_distance.distance < nearest_distance)
        {
            nearest_distance = link_and_distance.distance;
            nearest_link = link_and_distance.nearest_link;
            origin_link = &module_link;
        }
    }

    return {nearest_link, origin_link, nearest_distance};
}

std::shared_ptr<IModule> ShipBuilder::place_module(std::shared_ptr<IModule> selected_module)
{
    auto nearest_link_result = get_nearest_module_link_to_module(*selected_module);

    if (nearest_link_result.nearest_link == nullptr || nearest_link_result.distance > max_link_distance)
    {
        return nullptr;
    }
    else
    {
        nearest_link_result.nearest_link->link(*nearest_link_result.origin_link);
        agent.add_module(selected_module);
        agent.update_body();
        agent.register_actions();
        return selected_module;
    }
}

RenderData ShipBuilder::get_render_data(bool lightweight)
{
    return agent.get_render_data(lightweight);
}

TEST_CASE("ShipBuilder")
{
    b2World b2_world({0, 0});
    Random rng(0);
    IO io;
    io.set_resolution(1920, 1080);
    ShipBuilder ship_builder(b2_world, rng, io);

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

    SUBCASE("delete_module()")
    {
        SUBCASE("Correctly deletes a module")
        {
            auto gun_module = std::make_shared<GunModule>();
            auto point = screen_to_world_space({1030, 545}, io.get_resolution(), ship_builder.get_projection());
            gun_module->get_transform().p = {point.x, point.y};
            auto placed_module = ship_builder.place_module(gun_module);

            ship_builder.delete_module(placed_module);

            CHECK(ship_builder.get_agent()->get_modules().size() == 1);
        }

        SUBCASE("Won't delete base module")
        {
            auto base_module = ship_builder.get_agent()->get_modules()[0];
            ship_builder.delete_module(base_module);

            CHECK(ship_builder.get_agent()->get_modules().size() == 1);
        }
    }

    SUBCASE("place_module()")
    {
        SUBCASE("Correctly places a gun module")
        {
            auto gun_module = std::make_shared<GunModule>();
            auto point = screen_to_world_space({1030, 545}, io.get_resolution(), ship_builder.get_projection());
            gun_module->get_transform().p = {point.x, point.y};

            auto selected_module = ship_builder.place_module(gun_module);

            CHECK(selected_module == gun_module);
            REQUIRE(ship_builder.get_agent()->get_modules().size() == 2);

            CHECK(ship_builder.get_agent()->get_modules()[1]->get_transform().q.s == doctest::Approx(b2Rot(0).s));
            CHECK(ship_builder.get_agent()->get_modules()[1]->get_transform().q.c == doctest::Approx(b2Rot(0).c));
        }

        SUBCASE("Correctly places two gun modules")
        {
            auto gun_module_1 = std::make_shared<GunModule>();
            auto point = screen_to_world_space({1030, 545}, io.get_resolution(), ship_builder.get_projection());
            gun_module_1->get_transform().p = {point.x, point.y};
            ship_builder.place_module(gun_module_1);

            auto gun_module_2 = std::make_shared<GunModule>();
            point = screen_to_world_space({960, 625}, io.get_resolution(), ship_builder.get_projection());
            gun_module_2->get_transform().p = {point.x, point.y};
            auto selected_module = ship_builder.place_module(gun_module_2);

            CHECK(selected_module == gun_module_2);
            REQUIRE(ship_builder.get_agent()->get_modules().size() == 3);

            CHECK(ship_builder.get_agent()->get_modules()[2]->get_transform().q.s == doctest::Approx(b2Rot(0).s));
            CHECK(ship_builder.get_agent()->get_modules()[2]->get_transform().q.c == doctest::Approx(b2Rot(0).c));
        }

        SUBCASE("Can chain modules")
        {
            auto gun_module_1 = std::make_shared<GunModule>();
            auto point = screen_to_world_space({1030, 545}, io.get_resolution(), ship_builder.get_projection());
            gun_module_1->get_transform().p = {point.x, point.y};
            ship_builder.place_module(gun_module_1);

            auto gun_module_2 = std::make_shared<GunModule>();
            point = screen_to_world_space({1160, 545}, io.get_resolution(), ship_builder.get_projection());
            gun_module_2->get_transform().p = {point.x, point.y};
            auto selected_module = ship_builder.place_module(gun_module_2);

            CHECK(selected_module == gun_module_2);
            REQUIRE(ship_builder.get_agent()->get_modules().size() == 3);

            CHECK(ship_builder.get_agent()->get_modules()[2]->get_transform().q.s == doctest::Approx(b2Rot(0).s));
            CHECK(ship_builder.get_agent()->get_modules()[2]->get_transform().q.c == doctest::Approx(b2Rot(0).c));
        }
    }

    SUBCASE("get_module_at_screen_position()")
    {
        SUBCASE("Correctly selects the base module at 0, 0")
        {
            auto selected_module = ship_builder.get_module_at_screen_position({960, 540});
            auto base_module = ship_builder.get_agent()->get_modules()[0];

            CHECK(selected_module == base_module);
        }

        SUBCASE("Returns a nullptr if no module at exists at the point")
        {
            auto selected_module = ship_builder.get_module_at_screen_position({1000, 1000});

            CHECK(selected_module == nullptr);
        }
    }

    SUBCASE("get_nearest_module_link_to_world_position()")
    {
        SUBCASE("Selects the correct link under normal use")
        {
            auto selected_link = ship_builder.get_nearest_module_link_to_world_position({0, 10}).nearest_link;
            auto expected_link = &ship_builder.get_agent()->get_modules()[0]->get_module_links()[0];

            CHECK(selected_link == expected_link);
        }

        SUBCASE("Returns a nullptr if there are no available module links")
        {
            for (const auto &module : ship_builder.get_agent()->get_modules())
            {
                for (auto &module_link : module->get_module_links())
                {
                    module_link.linked = true;
                }
            }

            auto selected_link = ship_builder.get_nearest_module_link_to_world_position({0, 10}).nearest_link;

            CHECK(selected_link == nullptr);
        }
    }
}
}