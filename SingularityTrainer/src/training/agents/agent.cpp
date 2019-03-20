#include <memory>
#include <vector>
#include <sstream>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>

#include "random.h"
#include "training/agents/agent.h"
#include "training/environments/ienvironment.h"
#include "training/modules/base_module.h"
#include "training/modules/thruster_module.h"
#include "training/modules/gun_module.h"
#include "training/rigid_body.h"
#include "graphics/colors.h"
#include "utilities.h"
#include "random.h"

namespace SingularityTrainer
{
Agent::Agent(b2World &world, Random *rng, const nlohmann::json &json) : Agent(world, rng)
{
    load_json(json);
}

Agent::Agent(b2World &world, Random *rng, IEnvironment &environment) : Agent(world, rng)
{
    this->environment = &environment;
}

Agent::Agent(b2World &world, Random *rng, IEnvironment &environment, const nlohmann::json &json) : Agent(world, rng, environment)
{
    load_json(json);
}

Agent::Agent(b2World &world, Random *rng) : rng(rng), hp(0)
{
    // Rigid body
    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, b2Vec2_zero, world, this, RigidBody::ParentTypes::Agent);
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
}

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

void Agent::begin_contact(RigidBody *other) {}
void Agent::end_contact(RigidBody *other) {}

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

void Agent::load_json(const nlohmann::json &json)
{
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

nlohmann::json Agent::to_json() const
{
    return R"({
        "hello": "Woo"
    })"_json;
}

void Agent::update_body()
{
    // Destroy all fixtures
    for (b2Fixture *f = rigid_body->body->GetFixtureList(); f; f = f->GetNext())
    {
        rigid_body->body->DestroyFixture(f);
    }

    for (const auto &module : modules)
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
            rigid_body->body->CreateFixture(&fixture_def);
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

TEST_CASE("Agents can have modules added")
{
    Random rng(1);
    b2World b2_world({0, 0});
    Agent agent(b2_world, &rng);

    CHECK(agent.get_modules().size() == 0);

    auto module = std::make_shared<BaseModule>();
    agent.add_module(module);

    CHECK(agent.get_modules().size() == 1);

    SUBCASE("Added modules have their parent agent set correctly")
    {
        CHECK(agent.get_modules()[0]->get_agent() == &agent);
    }
}

TEST_CASE("Agents can be saved to Json and loaded back")
{
    Random rng(1);
    b2World b2_world({0, 0});
    Agent agent(b2_world, &rng);

    auto base_module = std::make_shared<BaseModule>();
    auto thruster_module = std::make_shared<ThrusterModule>();
    auto gun_module = std::make_shared<GunModule>();

    base_module->get_module_links()[0].link(&gun_module->get_module_links()[0]);
    gun_module->get_module_links()[2].link(&thruster_module->get_module_links()[0]);

    agent.add_module(base_module);
    agent.add_module(gun_module);
    agent.add_module(thruster_module);

    agent.update_body();
    agent.register_actions();

    auto json = agent.to_json();
    auto pretty_json = json.dump(2);
    INFO("Json: " << pretty_json);
    Agent loaded_agent(b2_world, &rng, json);

    SUBCASE("Agents have the same number of modules")
    {
        CHECK(loaded_agent.get_modules().size() == agent.get_modules().size());
    }

    SUBCASE("Agents have the same number of actions")
    {
        CHECK(loaded_agent.get_actions().size() == agent.get_actions().size());
    }
}
}
