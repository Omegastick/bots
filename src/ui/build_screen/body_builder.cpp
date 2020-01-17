#include <memory>

#include <Box2D/Box2D.h>
#include <doctest/doctest.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "body_builder.h"
#include "audio/audio_engine.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "misc/io.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "training/bodies/body.h"
#include "training/entities/bullet.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/module_link.h"
#include "training/rigid_body.h"
#include "misc/utilities.h"

namespace ai
{
const double max_link_distance = 1;

bool GetAllQueryCallback::ReportFixture(b2Fixture *fixture)
{
    fixtures.push_back(fixture);
    return true;
}

BodyBuilder::BodyBuilder(std::unique_ptr<Body> body, std::unique_ptr<b2World> world, IO &io)
    : body(std::move(body)),
      io(io),
      projection(glm::ortho(-9.6f, 9.6f, -5.4f, 5.4f)),
      selected_module(nullptr),
      world(std::move(world))
{
    auto base_module = std::make_shared<BaseModule>();
    this->body->add_module(base_module);
    this->body->update_body();
}

void BodyBuilder::delete_module(IModule *module)
{
    if (module == body->get_modules()[0].get())
    {
        spdlog::error("Can't delete base module");
        return;
    }

    body->unlink_module(module);
    body->update_body();

    if (module == selected_module)
    {
        selected_module = nullptr;
    }
}

std::shared_ptr<IModule> BodyBuilder::get_module_at_screen_position(glm::vec2 point)
{
    point = screen_to_world_space(point, io.get_resolutionf(), projection);

    GetAllQueryCallback query_callback;
    world->QueryAABB(&query_callback, {{point.x, point.y},
                                       {point.x, point.y}});

    auto collisions = query_callback.get_collisions();
    if (collisions.size() == 0)
    {
        return nullptr;
    }

    auto fixture = collisions[0];
    auto rigid_body = static_cast<RigidBody *>(fixture->GetBody()->GetUserData());
    if (rigid_body->parent_type != RigidBody::ParentTypes::Body)
    {
        return nullptr;
    }

    return *static_cast<std::shared_ptr<IModule> *>(fixture->GetUserData());
}

NearestModuleLinkResult BodyBuilder::get_nearest_module_link_to_world_position(glm::vec2 point)
{
    b2Vec2 b2_point(point.x, point.y);

    ModuleLink *nearest_link = nullptr;
    double nearest_distance = INFINITY;

    for (const auto &module : body->get_modules())
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

NearestModuleLinkResult BodyBuilder::get_nearest_module_link_to_module(IModule &module)
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

std::shared_ptr<IModule> BodyBuilder::place_module(std::shared_ptr<IModule> selected_module)
{
    auto nearest_link_result = get_nearest_module_link_to_module(*selected_module);

    if (nearest_link_result.nearest_link == nullptr || nearest_link_result.distance > max_link_distance)
    {
        return nullptr;
    }

    nearest_link_result.nearest_link->link(*nearest_link_result.origin_link);
    body->add_module(selected_module);
    body->update_body();
    body->register_actions();
    return selected_module;
}

void BodyBuilder::select_module(const IModule *module)
{
    selected_module = module;
}

void BodyBuilder::draw(Renderer &renderer, bool lightweight)
{
    renderer.set_view(projection);

    body->draw(renderer, lightweight);

    if (selected_module != nullptr)
    {
        Sprite selected_marker;
        selected_marker.texture = "square";
        selected_marker.color = cl_white;
        selected_marker.transform.set_scale({1.1, 1.1});
        auto b2_transform = selected_module->get_global_transform();
        selected_marker.transform.set_position({b2_transform.p.x, b2_transform.p.y});
        selected_marker.transform.set_rotation(b2_transform.q.GetAngle());
        renderer.draw(selected_marker);
    }
}

TEST_CASE("BodyBuilder")
{
    Random rng(0);
    MockAudioEngine audio_engine;
    BulletFactory bullet_factory(audio_engine);
    ModuleFactory module_factory(bullet_factory, rng);
    BodyFactory body_factory(module_factory, rng);
    IO io;
    io.set_resolution(1920, 1080);
    auto world = std::make_unique<b2World>(b2Vec2_zero);
    auto body = body_factory.make(*world, rng);
    BodyBuilder body_builder(std::move(body), std::move(world), io);

    SUBCASE("Starts with only a single base module")
    {
        auto body = &body_builder.get_body();
        auto modules = body->get_modules();

        DOCTEST_CHECK(modules.size() == 1);

        auto transform = modules[0]->get_global_transform();
        DOCTEST_CHECK(transform.p.Length() == 0.f);
        DOCTEST_CHECK(transform.q.GetAngle() == 0.f);

        auto json = modules[0]->to_json();
        DOCTEST_CHECK(json["type"] == "base_module");
    }

    SUBCASE("delete_module()")
    {
        SUBCASE("Correctly deletes a module")
        {
            auto gun_module = module_factory.make("gun_module");
            auto point = screen_to_world_space({1030, 545},
                                               io.get_resolution(),
                                               body_builder.get_projection());
            gun_module->get_transform().p = {point.x, point.y};
            auto placed_module = body_builder.place_module(gun_module);

            body_builder.delete_module(placed_module.get());

            DOCTEST_CHECK(body_builder.get_body().get_modules().size() == 1);
        }

        SUBCASE("Won't delete base module")
        {
            auto base_module = body_builder.get_body().get_modules()[0];
            body_builder.delete_module(base_module.get());

            DOCTEST_CHECK(body_builder.get_body().get_modules().size() == 1);
        }
    }

    SUBCASE("place_module()")
    {
        SUBCASE("Correctly places a gun module")
        {
            auto gun_module = module_factory.make("gun_module");
            auto point = screen_to_world_space({1030, 545}, io.get_resolution(), body_builder.get_projection());
            gun_module->get_transform().p = {point.x, point.y};

            auto selected_module = body_builder.place_module(gun_module);

            DOCTEST_CHECK(selected_module == gun_module);
            REQUIRE(body_builder.get_body().get_modules().size() == 2);

            DOCTEST_CHECK(body_builder.get_body().get_modules()[1]->get_transform().q.s == doctest::Approx(b2Rot(0).s));
            DOCTEST_CHECK(body_builder.get_body().get_modules()[1]->get_transform().q.c == doctest::Approx(b2Rot(0).c));
        }

        SUBCASE("Correctly places two gun modules")
        {
            auto gun_module_1 = module_factory.make("gun_module");
            auto point = screen_to_world_space({1030, 545}, io.get_resolution(), body_builder.get_projection());
            gun_module_1->get_transform().p = {point.x, point.y};
            body_builder.place_module(gun_module_1);

            auto gun_module_2 = module_factory.make("gun_module");
            point = screen_to_world_space({960, 625}, io.get_resolution(), body_builder.get_projection());
            gun_module_2->get_transform().p = {point.x, point.y};
            auto selected_module = body_builder.place_module(gun_module_2);

            DOCTEST_CHECK(selected_module == gun_module_2);
            REQUIRE(body_builder.get_body().get_modules().size() == 3);

            DOCTEST_CHECK(body_builder.get_body().get_modules()[2]->get_transform().q.s == doctest::Approx(b2Rot(0).s));
            DOCTEST_CHECK(body_builder.get_body().get_modules()[2]->get_transform().q.c == doctest::Approx(b2Rot(0).c));
        }

        SUBCASE("Can chain modules")
        {
            auto gun_module_1 = module_factory.make("gun_module");
            auto point = screen_to_world_space({1030, 545}, io.get_resolution(), body_builder.get_projection());
            gun_module_1->get_transform().p = {point.x, point.y};
            body_builder.place_module(gun_module_1);

            auto gun_module_2 = module_factory.make("gun_module");
            point = screen_to_world_space({1160, 545}, io.get_resolution(), body_builder.get_projection());
            gun_module_2->get_transform().p = {point.x, point.y};
            auto selected_module = body_builder.place_module(gun_module_2);

            DOCTEST_CHECK(selected_module == gun_module_2);
            REQUIRE(body_builder.get_body().get_modules().size() == 3);

            DOCTEST_CHECK(body_builder.get_body().get_modules()[2]->get_transform().q.s == doctest::Approx(b2Rot(0).s));
            DOCTEST_CHECK(body_builder.get_body().get_modules()[2]->get_transform().q.c == doctest::Approx(b2Rot(0).c));
        }
    }

    SUBCASE("get_module_at_screen_position()")
    {
        SUBCASE("Correctly selects the base module at 0, 0")
        {
            auto selected_module = body_builder.get_module_at_screen_position({960, 540});
            auto base_module = body_builder.get_body().get_modules()[0];

            DOCTEST_CHECK(selected_module == base_module);
        }

        SUBCASE("Returns a nullptr if no module at exists at the point")
        {
            auto selected_module = body_builder.get_module_at_screen_position({1000, 1000});

            DOCTEST_CHECK(selected_module == nullptr);
        }
    }

    SUBCASE("get_nearest_module_link_to_world_position()")
    {
        SUBCASE("Selects the correct link under normal use")
        {
            auto selected_link = body_builder.get_nearest_module_link_to_world_position({0, 10}).nearest_link;
            auto expected_link = &body_builder.get_body().get_modules()[0]->get_module_links()[0];

            DOCTEST_CHECK(selected_link == expected_link);
        }

        SUBCASE("Returns a nullptr if there are no available module links")
        {
            for (const auto &module : body_builder.get_body().get_modules())
            {
                for (auto &module_link : module->get_module_links())
                {
                    module_link.linked = true;
                }
            }

            auto selected_link = body_builder.get_nearest_module_link_to_world_position({0, 10}).nearest_link;

            DOCTEST_CHECK(selected_link == nullptr);
        }
    }
}
}