#include <memory>

#include <Box2D/Box2D.h>
#include <doctest/doctest.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "ui/build_screen/ship_builder.h"
#include "io.h"
#include "random.h"
#include "training/agents/agent.h"
#include "training/modules/base_module.h"
#include "training/modules/module_link.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
bool GetAllQueryCallback::ReportFixture(b2Fixture *fixture)
{
    fixtures.push_back(fixture);
    return true;
}

ShipBuilder::ShipBuilder(b2World &b2_world, Random &rng, IO &io)
    : agent(std::make_unique<Agent>(b2_world, &rng)),
      io(&io),
      b2_world(&b2_world),
      projection(glm::ortho(-9.6f, 9.6f, -5.4f, 5.4f))
{
    auto base_module = std::make_shared<BaseModule>();
    agent->add_module(base_module);
    agent->update_body();
}

IModule *ShipBuilder::get_module_at_point(glm::vec2 point)
{
    auto resolution = static_cast<glm::vec2>(io->get_resolution());
    point.y = resolution.y - point.y;
    point = point - (resolution / 2.f);
    point = point / resolution;
    point = glm::vec2(point.x * (1. / projection[0][0]), point.y * (1. / projection[1][1]));

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

    return static_cast<IModule *>(fixture->GetUserData());
}

ModuleLinkAndDistance ShipBuilder::get_nearest_module_link(glm::vec2 point)
{
    auto resolution = static_cast<glm::vec2>(io->get_resolution());
    point.y = resolution.y - point.y;
    point = point - (resolution / 2.f);
    point = point / resolution;
    point = glm::vec2(point.x * (1. / projection[0][0]), point.y * (1. / projection[1][1]));
    b2Vec2 b2_point(point.x, point.y);

    ModuleLink *closest_link = nullptr;
    double closest_distance = INFINITY;

    for (const auto &module : agent->get_modules())
    {
        for (auto &module_link : module->get_module_links())
        {
            if (module_link.linked)
            {
                continue;
            }
            double distance = (module_link.transform.p - b2_point).LengthSquared();
            if (distance < closest_distance)
            {
                closest_link = &module_link;
                closest_distance = distance;
            }
        }
    }

    return {closest_link, closest_distance};
}

void ShipBuilder::update() {}

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

    SUBCASE("get_module_at_point()")
    {
        SUBCASE("Correctly selects the base module at 0, 0")
        {
            auto selected_module = ship_builder.get_module_at_point({960, 540});
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