#include <memory>
#include <sstream>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "misc/random.h"
#include "training/agents/agent.h"
#include "training/environments/ienvironment.h"
#include "training/modules/base_module.h"
#include "training/modules/thruster_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/laser_sensor_module.h"
#include "training/rigid_body.h"
#include "graphics/colors.h"
#include "misc/utilities.h"
#include "misc/random.h"

namespace SingularityTrainer
{
static const std::string schema_version = "v1alpha3";

Agent::Agent(Random &rng) : hp(0), rng(&rng)
{
}

Agent::Agent(Agent &&other)
{
    (*this) = std::move(other);
}

Agent &Agent::operator=(Agent &&other)
{
    if (this != &other)
    {
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
            module->set_agent(this);
        }
    }

    return *this;
}

void Agent::act(std::vector<int> action_flags)
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

void Agent::add_module(const std::shared_ptr<IModule> module)
{
    module->set_agent(this);
    modules.push_back(module);
    register_actions();
    update_body();
}

void Agent::begin_contact(RigidBody * /*other*/) {}
void Agent::end_contact(RigidBody * /*other*/) {}

std::vector<float> Agent::get_observation()
{
    std::vector<float> observation;
    for (const auto &module : modules)
    {
        std::vector<float> sensor_reading = module->get_sensor_reading();
        observation.insert(observation.end(), sensor_reading.begin(), sensor_reading.end());
    }
    return observation;
}

int Agent::get_input_count() const
{
    int total_inputs = 0;
    for (const auto &action : actions)
    {
        total_inputs += action->flag_count;
    }
    return total_inputs;
}

RenderData Agent::get_render_data(bool lightweight)
{
    RenderData render_data;
    for (const auto &module : modules)
    {
        auto x = module->get_render_data(lightweight);
        render_data.append(x);
    }

    std::stringstream hp_stream;
    hp_stream << hp;
    Text text;
    text.font = "roboto-16";
    text.text = hp_stream.str();
    text.color = cl_white;
    b2Vec2 position = rigid_body->body->GetPosition();
    text.set_position({position.x, position.y});
    text.set_scale({0.1, 0.1});
    render_data.texts.push_back(text);

    return render_data;

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
    //     for (const const auto &module : modules)
    //     {
    //         b2Transform module_transform = module->get_global_transform();
    //         for (const const auto &module_link : module->module_links)
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

void Agent::init_rigid_body()
{
    rigid_body->parent = this;
    rigid_body->parent_type = RigidBody::ParentTypes::Agent;
}

void Agent::load_json(const nlohmann::json &json)
{
    if (json["schema"] != schema_version)
    {
        auto error_text = fmt::format("Invalid Agent Json schema version: {}. Excpected: {}",
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
}

void Agent::recurse_json_modules(const nlohmann::json &module_json, IModule *parent_module, int parent_link, int child_link)
{
    std::shared_ptr<IModule> module;
    if (module_json["type"] == "base")
    {
        module = std::make_shared<BaseModule>();
    }
    else if (module_json["type"] == "gun")
    {
        module = std::make_shared<GunModule>();
    }
    else if (module_json["type"] == "thruster")
    {
        module = std::make_shared<ThrusterModule>();
    }
    else if (module_json["type"] == "laser_sensor")
    {
        module = std::make_shared<LaserSensorModule>();
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

void Agent::register_actions()
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

void Agent::set_rigid_body(std::unique_ptr<RigidBody> rigid_body)
{
    this->rigid_body = std::move(rigid_body);
    init_rigid_body();
}

nlohmann::json Agent::to_json() const
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

void Agent::unlink_module(std::shared_ptr<IModule> module)
{
    ModuleLink *main_link;
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

    main_link->pair_link->linked = false;
    main_link->pair_link->pair_link = nullptr;
    main_link->pair_link->is_parent = false;
    main_link->pair_link->linked_module = nullptr;

    main_link->linked = false;
    main_link->pair_link = nullptr;
    main_link->linked_module = nullptr;

    modules.erase(std::find_if(modules.begin(), modules.end(), [module](const auto &i) { return module.get() == i.get(); }));
}

void Agent::update_body()
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

void Agent::hit(float damage)
{
    hp -= damage;
}

void Agent::reset()
{
    hp = 10;
}

TEST_CASE("Agent")
{
    Random rng(1);
    b2World b2_world({0, 0});
    auto rigid_body = std::make_unique<RigidBody>(b2_dynamicBody,
                                                  b2Vec2_zero,
                                                  b2_world,
                                                  nullptr,
                                                  RigidBody::ParentTypes::Agent);

    SUBCASE("Can have modules added")
    {
        Agent agent(rng);
        agent.set_rigid_body(std::move(rigid_body));

        DOCTEST_CHECK(agent.get_modules().size() == 0);

        auto module = std::make_shared<BaseModule>();
        agent.add_module(module);

        DOCTEST_CHECK(agent.get_modules().size() == 1);

        SUBCASE("Added modules have their parent agent set correctly")
        {
            DOCTEST_CHECK(agent.get_modules()[0]->get_agent() == &agent);
        }
    }

    SUBCASE("Can be saved to Json and loaded back")
    {
        Agent agent(rng);
        agent.set_rigid_body(std::move(rigid_body));

        auto base_module = std::make_shared<BaseModule>();
        auto thruster_module = std::make_shared<ThrusterModule>();
        auto gun_module = std::make_shared<GunModule>();

        base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);
        gun_module->get_module_links()[2].link(thruster_module->get_module_links()[0]);

        agent.add_module(base_module);
        agent.add_module(gun_module);
        agent.add_module(thruster_module);

        agent.update_body();
        agent.register_actions();

        auto json = agent.to_json();
        auto pretty_json = json.dump(2);
        INFO("Json: " << pretty_json);
        rigid_body = std::make_unique<RigidBody>(b2_dynamicBody,
                                                 b2Vec2_zero,
                                                 b2_world,
                                                 nullptr,
                                                 RigidBody::ParentTypes::Agent);
        Agent loaded_agent(rng);
        loaded_agent.set_rigid_body(std::move(rigid_body));
        loaded_agent.load_json(json);

        SUBCASE("Loaded Agents have the same number of modules")
        {
            DOCTEST_CHECK(loaded_agent.get_modules().size() == agent.get_modules().size());
        }

        SUBCASE("Loaded Agents have the same number of actions")
        {
            DOCTEST_CHECK(loaded_agent.get_actions().size() == agent.get_actions().size());
        }
    }

    SUBCASE("With no modules are converted to Json correctly")
    {
        Agent agent(rng);
        agent.set_rigid_body(std::move(rigid_body));

        auto json = agent.to_json();
        auto pretty_json = json.dump(2);
        INFO("Json :" << pretty_json);

        DOCTEST_CHECK(json["base_module"] == nullptr);
    }

    SUBCASE("Load Json errors on invalid schema version")
    {
        auto json = "{\"schema\": \"bad_schema\"}"_json;

        Agent agent(rng);
        CHECK_THROWS(agent.load_json(json));
    }

    SUBCASE("get_input_count() returns correct number of inputs")
    {
        SUBCASE("0")
        {
            Agent agent(rng);
            agent.set_rigid_body(std::move(rigid_body));

            int num_inputs = agent.get_input_count();
            DOCTEST_CHECK(num_inputs == 0);
        }

        SUBCASE("1")
        {
            Agent agent(rng);
            agent.set_rigid_body(std::move(rigid_body));

            auto base_module = std::make_shared<BaseModule>();
            auto gun_module = std::make_shared<GunModule>();

            base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);

            agent.add_module(base_module);
            agent.add_module(gun_module);

            int num_inputs = agent.get_input_count();
            DOCTEST_CHECK(num_inputs == 1);
        }

        SUBCASE("2")
        {
            Agent agent(rng);
            agent.set_rigid_body(std::move(rigid_body));

            auto base_module = std::make_shared<BaseModule>();
            auto gun_module = std::make_shared<GunModule>();
            auto gun_module_2 = std::make_shared<GunModule>();

            base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);
            base_module->get_module_links()[1].link(gun_module->get_module_links()[0]);

            agent.add_module(base_module);
            agent.add_module(gun_module);
            agent.add_module(gun_module_2);

            int num_inputs = agent.get_input_count();
            DOCTEST_CHECK(num_inputs == 2);
        }
    }

    SUBCASE("unlink()")
    {
        SUBCASE("Removes the module from the Agent")
        {
            Agent agent(rng);
            agent.set_rigid_body(std::move(rigid_body));

            auto base_module = std::make_shared<BaseModule>();
            auto gun_module = std::make_shared<GunModule>();

            base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);

            agent.add_module(base_module);
            agent.add_module(gun_module);

            agent.update_body();
            agent.register_actions();

            agent.unlink_module(gun_module);
            agent.update_body();
            agent.register_actions();

            DOCTEST_CHECK(agent.get_modules().size() == 1);
        }

        SUBCASE("Throws when trying to remove module with children")
        {
            Agent agent(rng);
            agent.set_rigid_body(std::move(rigid_body));

            auto base_module = std::make_shared<BaseModule>();
            auto thruster_module = std::make_shared<ThrusterModule>();
            auto gun_module = std::make_shared<GunModule>();

            base_module->get_module_links()[0].link(gun_module->get_module_links()[0]);
            gun_module->get_module_links()[2].link(thruster_module->get_module_links()[0]);

            agent.add_module(base_module);
            agent.add_module(gun_module);
            agent.add_module(thruster_module);

            agent.update_body();
            agent.register_actions();

            CHECK_THROWS(agent.unlink_module(gun_module));
        }
    }
}
}
