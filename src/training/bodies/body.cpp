#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <glm/vec4.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "misc/random.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "training/bodies/body.h"
#include "training/events/effect_triggered.h"
#include "training/environments/ienvironment.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/square_hull.h"
#include "training/modules/thruster_module.h"
#include "training/rigid_body.h"
#include "misc/utilities.h"
#include "misc/random.h"

namespace SingularityTrainer
{
static const std::string schema_version = "v1alpha4";

Body::Body(Random &rng) : hp(0), rng(&rng)
{
}

Body::Body(Body &&other)
{
    (*this) = std::move(other);
}

Body &Body::operator=(Body &&other)
{
    if (this == &other)
    {
        return *this;
    }

    modules = std::move(other.modules);
    actions = std::move(other.actions);
    rigid_body = std::move(other.rigid_body);
    debug_draw = other.debug_draw;
    other.debug_draw = false;
    rng = other.rng;
    other.rng = nullptr;
    hp = other.hp;
    other.hp = 0;
    environment = other.environment;
    other.environment = nullptr;
    name = other.name;
    other.name = "";

    for (const auto &module : modules)
    {
        module->set_body(this);
    }

    return *this;
}

void Body::act(std::vector<int> action_flags)
{
    for (const auto &module : modules)
    {
        module->update();
    }

    int current_position = 0;
    for (const auto &action : actions)
    {
        int next_position = current_position + action->flag_count;
        action->act(std::vector<int>(action_flags.begin() + current_position, action_flags.begin() + next_position));
        current_position = next_position;
    }
}

void Body::add_module(const std::shared_ptr<IModule> module)
{
    module->set_body(this);
    modules.push_back(module);
    register_actions();
    if (rigid_body != nullptr)
    {
        update_body();
    }
}

void Body::begin_contact(RigidBody * /*other*/) {}
void Body::end_contact(RigidBody * /*other*/) {}

std::vector<float> Body::get_observation() const
{
    std::vector<float> observation;
    for (const auto &module : modules)
    {
        std::vector<float> sensor_reading = module->get_sensor_reading();
        observation.insert(observation.end(), sensor_reading.begin(), sensor_reading.end());
    }
    return observation;
}

int Body::get_input_count() const
{
    int total_inputs = 0;
    for (const auto &action : actions)
    {
        total_inputs += action->flag_count;
    }
    return total_inputs;
}

void Body::draw(Renderer &renderer, bool lightweight)
{
    for (const auto &module : modules)
    {
        module->draw(renderer, lightweight);
    }

    std::stringstream hp_stream;
    hp_stream << hp;
    Text text;
    text.font = "roboto-16";
    text.text = hp_stream.str();
    text.color = cl_white;
    b2Vec2 position = rigid_body->body->GetPosition();
    text.transform.set_position({position.x, position.y});
    text.transform.set_scale({0.1, 0.1});
    renderer.draw(text);

    // if (debug_draw)
    // {
    //     b2Transform transform = rigid_body->body->GetTransform();

    //     // Draw modules
    //     for (b2Fixture *fixture = rigid_body->body->GetFixtureList(); fixture; fixture = fixture->GetNext())
    //     {
    //         b2PolygonShape *b2_shape = (b2PolygonShape *)fixture->GetShape();
    //         sf::ConvexShape screen_shape(b2_shape->GetVertexCount());
    //         screen_shape.setOutlineColor(sf::Color::Green);
    //         screen_shape.setOutlineThickness(0.02);
    //         screen_shape.setFillColor(sf::Color::Transparent);
    //         for (int i = 0; i < b2_shape->m_count; ++i)
    //         {
    //             b2Vec2 vertex_position = b2Mul(transform, b2_shape->GetVertex(i));
    //             screen_shape.setPoint(i, sf::Vector2f(vertex_position.x, vertex_position.y));
    //         }
    //         render_target.draw(screen_shape);
    //     }

    //     // Draw module links
    //     for (const auto &module : modules)
    //     {
    //         b2Transform module_transform = module->get_global_transform();
    //         for (const auto &module_link : module->module_links)
    //         {
    //             // Transform
    //             b2Transform link_transform = b2Mul(module_transform, module_link.transform);

    //             // Circle
    //             sf::CircleShape circle(0.1);
    //             circle.setOrigin(0.1, 0.1);
    //             if (module_link.linked)
    //             {
    //                 circle.setFillColor(sf::Color(0, 255, 0, 100));
    //             }
    //             else
    //             {
    //                 circle.setFillColor(sf::Color(125, 125, 125, 100));
    //             }
    //             circle.setPosition(link_transform.p.x, link_transform.p.y);
    //             render_target.draw(circle);

    //             // Direction line
    //             sf::RectangleShape line(sf::Vector2f(0.02, -0.1));
    //             line.setOrigin(0.01, 0);
    //             line.setFillColor(sf::Color::Red);
    //             line.setRotation(rad_to_deg(link_transform.q.GetAngle()));
    //             line.setPosition(link_transform.p.x, link_transform.p.y);
    //             render_target.draw(line);
    //         }
    //     }
    // }
}

void Body::init_rigid_body()
{
    rigid_body->parent = this;
    rigid_body->parent_type = RigidBody::ParentTypes::Body;
}

void Body::load_json(const nlohmann::json &json)
{
    if (json["schema"] != schema_version)
    {
        auto error_text = fmt::format("Invalid Body Json schema version: {}. Excpected: {}",
                                      static_cast<std::string>(json["schema"]),
                                      schema_version);
        throw std::runtime_error(error_text);
    }

    if (json["name"] == nullptr)
    {
        throw std::runtime_error("Malformed Json - No name");
    }

    name = json["name"];
    modules = {};

    if (json["base_module"] != nullptr)
    {
        recurse_json_modules(json["base_module"]);
    }

    update_body();
    register_actions();

    hp = 10;
}

void Body::recurse_json_modules(const nlohmann::json &module_json, IModule *parent_module, int parent_link, int child_link)
{
    std::shared_ptr<IModule> module;
    if (module_json["type"] == "base")
    {
        module = std::make_shared<BaseModule>();
    }
    else if (module_json["type"] == "gun")
    {
        module = std::make_shared<GunModule>(*rng);
    }
    else if (module_json["type"] == "laser_sensor")
    {
        module = std::make_shared<LaserSensorModule>(module_json["laser_count"]);
    }
    else if (module_json["type"] == "square_hull")
    {
        module = std::make_shared<SquareHull>();
    }
    else if (module_json["type"] == "thruster")
    {
        module = std::make_shared<ThrusterModule>();
    }

    add_module(module);

    if (parent_module != nullptr)
    {
        parent_module->get_module_links()[parent_link].link(module->get_module_links()[child_link]);
    }

    for (unsigned int i = 0; i < module_json["links"].size(); ++i)
    {
        if (module_json["links"][i] != nullptr)
        {
            recurse_json_modules(
                module_json["links"][i]["child"],
                module.get(),
                i,
                module_json["links"][i]["child_link"]);
        }
    }
}

void Body::register_actions()
{
    actions = std::vector<IAction *>();
    for (const auto &module : modules)
    {
        for (const auto &action : module->get_actions())
        {
            actions.push_back(action.get());
        }
    }
}

void Body::set_rigid_body(std::unique_ptr<RigidBody> rigid_body)
{
    this->rigid_body = std::move(rigid_body);
    init_rigid_body();
}

void Body::sub_update()
{
    for (auto &module : modules)
    {
        module->sub_update();
    }
}

nlohmann::json Body::to_json() const
{
    auto json = nlohmann::json::object();

    json["schema"] = schema_version;
    json["name"] = name;
    if (modules.size() > 0)
    {
        json["base_module"] = modules[0]->get_root_module()->to_json();
    }
    else
    {
        json["base_module"] = nullptr;
    }

    int observation_count = 0;
    for (const auto &module : modules)
    {
        observation_count += module->get_observation_count();
    }
    json["num_observations"] = observation_count;
    json["num_actions"] = get_input_count();

    return json;
}

void Body::unlink_module(IModule *module)
{
    ModuleLink *main_link = nullptr;
    for (auto &link : module->get_module_links())
    {
        if (link.linked)
        {
            if (link.is_parent)
            {
                throw std::runtime_error("Can't unlink module with children");
            }
            main_link = &link;
        }
    }

    if (main_link == nullptr)
    {
        throw std::runtime_error("Module has no parent links");
    }

    main_link->pair_link->linked = false;
    main_link->pair_link->pair_link = nullptr;
    main_link->pair_link->is_parent = false;
    main_link->pair_link->linked_module = nullptr;

    main_link->linked = false;
    main_link->pair_link = nullptr;
    main_link->linked_module = nullptr;

    modules.erase(std::find_if(modules.begin(), modules.end(), [module](const auto &i) { return module == i.get(); }));
}

void Body::update_body()
{
    // Destroy all fixtures
    auto fixture = rigid_body->body->GetFixtureList();
    while (fixture)
    {
        auto next_fixture = fixture->GetNext();
        rigid_body->body->DestroyFixture(fixture);
        fixture = next_fixture;
    }

    for (auto &module : modules)
    {
        // Copy the module's screen_shape
        // It's important we leave the original intact in case we need to do this again
        std::vector<b2PolygonShape> screen_shapes = module->get_shapes();
        for (auto &screen_shape : screen_shapes)
        {
            std::vector<b2Vec2> points;
            points.resize(screen_shape.m_count);

            // Apply transform to all points in screen_shape
            for (int i = 0; i < screen_shape.m_count; ++i)
            {
                points[i] = b2Mul(module->get_transform(), screen_shape.m_vertices[i]);
            }
            screen_shape.Set(&points[0], screen_shape.m_count);

            // Create the fixture
            b2FixtureDef fixture_def;
            fixture_def.shape = &screen_shape;
            fixture_def.density = 1;
            fixture_def.friction = 1;
            auto fixture = rigid_body->body->CreateFixture(&fixture_def);
            fixture->SetUserData(&module);
        }
    }
}

void Body::hit(float damage)
{
    hp -= damage;
}

void Body::reset()
{
    hp = 10;
}

void Body::set_color(glm::vec4 color)
{
    for (auto &module : modules)
    {
        module->set_color(color);
    }
}

TEST_CASE("Body")
{
    Random rng(1);
    b2World b2_world({0, 0});
    auto rigid_body = std::make_unique<RigidBody>(b2_dynamicBody,
                                                  b2Vec2_zero,
                                                  b2_world,
                                                  nullptr,
                                                  RigidBody::ParentTypes::Body);

    SUBCASE("Can have modules added")
    {
        Body body(rng);
        body.set_rigid_body(std::move(rigid_body));

        DOCTEST_CHECK(body.get_modules().size() == 0);

        auto module = std::make_shared<BaseModule>();
        body.add_module(module);

        DOCTEST_CHECK(body.get_modules().size() == 1);

        SUBCASE("Added modules have their parent body set correctly")
        {
            DOCTEST_CHECK(body.get_modules()[0]->get_body() == &body);
        }
    }

    SUBCASE("Can be saved to Json and loaded back")
    {
        Body body(rng);
        body.set_rigid_body(std::move(rigid_body));

        auto base_module = std::make_shared<BaseModule>();
        auto thruster_module = std::make_shared<ThrusterModule>();
        auto gun_module = std::make_shared<GunModule>(rng);

        base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);
        gun_module->get_module_links()[2].link(thruster_module->get_module_links()[0]);

        body.add_module(base_module);
        body.add_module(gun_module);
        body.add_module(thruster_module);

        body.update_body();
        body.register_actions();

        auto json = body.to_json();
        auto pretty_json = json.dump(2);
        INFO("Json: " << pretty_json);
        rigid_body = std::make_unique<RigidBody>(b2_dynamicBody,
                                                 b2Vec2_zero,
                                                 b2_world,
                                                 nullptr,
                                                 RigidBody::ParentTypes::Body);
        Body loaded_body(rng);
        loaded_body.set_rigid_body(std::move(rigid_body));
        loaded_body.load_json(json);

        SUBCASE("Loaded Bodys have the same number of modules")
        {
            DOCTEST_CHECK(loaded_body.get_modules().size() == body.get_modules().size());
        }

        SUBCASE("Loaded Bodys have the same number of actions")
        {
            DOCTEST_CHECK(loaded_body.get_actions().size() == body.get_actions().size());
        }
    }

    SUBCASE("With no modules are converted to Json correctly")
    {
        Body body(rng);
        body.set_rigid_body(std::move(rigid_body));

        auto json = body.to_json();
        auto pretty_json = json.dump(2);
        INFO("Json :" << pretty_json);

        DOCTEST_CHECK(json["base_module"] == nullptr);
    }

    SUBCASE("Load Json errors on invalid schema version")
    {
        auto json = "{\"schema\": \"bad_schema\"}"_json;

        Body body(rng);
        CHECK_THROWS(body.load_json(json));
    }

    SUBCASE("get_input_count() returns correct number of inputs")
    {
        SUBCASE("0")
        {
            Body body(rng);
            body.set_rigid_body(std::move(rigid_body));

            int num_inputs = body.get_input_count();
            DOCTEST_CHECK(num_inputs == 0);
        }

        SUBCASE("1")
        {
            Body body(rng);
            body.set_rigid_body(std::move(rigid_body));

            auto base_module = std::make_shared<BaseModule>();
            auto gun_module = std::make_shared<GunModule>(rng);

            base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);

            body.add_module(base_module);
            body.add_module(gun_module);

            int num_inputs = body.get_input_count();
            DOCTEST_CHECK(num_inputs == 1);
        }

        SUBCASE("2")
        {
            Body body(rng);
            body.set_rigid_body(std::move(rigid_body));

            auto base_module = std::make_shared<BaseModule>();
            auto gun_module = std::make_shared<GunModule>(rng);
            auto gun_module_2 = std::make_shared<GunModule>(rng);

            base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);
            base_module->get_module_links()[1].link(gun_module->get_module_links()[0]);

            body.add_module(base_module);
            body.add_module(gun_module);
            body.add_module(gun_module_2);

            int num_inputs = body.get_input_count();
            DOCTEST_CHECK(num_inputs == 2);
        }
    }

    SUBCASE("unlink()")
    {
        SUBCASE("Removes the module from the Body")
        {
            Body body(rng);
            body.set_rigid_body(std::move(rigid_body));

            auto base_module = std::make_shared<BaseModule>();
            auto gun_module = std::make_shared<GunModule>(rng);

            base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);

            body.add_module(base_module);
            body.add_module(gun_module);

            body.update_body();
            body.register_actions();

            body.unlink_module(gun_module.get());
            body.update_body();
            body.register_actions();

            DOCTEST_CHECK(body.get_modules().size() == 1);
        }

        SUBCASE("Throws when trying to remove module with children")
        {
            Body body(rng);
            body.set_rigid_body(std::move(rigid_body));

            auto base_module = std::make_shared<BaseModule>();
            auto thruster_module = std::make_shared<ThrusterModule>();
            auto gun_module = std::make_shared<GunModule>(rng);

            base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);
            gun_module->get_module_links()[2].link(thruster_module->get_module_links()[0]);

            body.add_module(base_module);
            body.add_module(gun_module);
            body.add_module(thruster_module);

            body.update_body();
            body.register_actions();

            CHECK_THROWS(body.unlink_module(gun_module.get()));
        }
    }
}
}
