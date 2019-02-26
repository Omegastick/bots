#include <memory>
#include <vector>

#include <Box2D/Box2D.h>

#include "random.h"
#include "training/agents/test_agent.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/thruster_module.h"
#include "training/rigid_body.h"
#include "utilities.h"

namespace SingularityTrainer
{
TestAgent::TestAgent(b2World &world, Random *rng)
{
    // Rigid body
    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, b2Vec2_zero, world, this, RigidBody::ParentTypes::Agent);

    // Instantiate modules
    std::unique_ptr<BaseModule> base_module = std::make_unique<BaseModule>(*rigid_body->body, this);
    std::unique_ptr<GunModule> gun_module_right = std::make_unique<GunModule>(*rigid_body->body, this);
    std::unique_ptr<GunModule> gun_module_left = std::make_unique<GunModule>(*rigid_body->body, this);
    std::unique_ptr<ThrusterModule> thruster_module_left = std::make_unique<ThrusterModule>(*rigid_body->body, this);
    std::unique_ptr<ThrusterModule> thruster_module_right = std::make_unique<ThrusterModule>(*rigid_body->body, this);
    std::unique_ptr<LaserSensorModule> laser_sensor_module = std::make_unique<LaserSensorModule>(*rigid_body->body, this);

    // Connect modules
    base_module->module_links[1].link(&gun_module_right->module_links[2]);
    base_module->module_links[3].link(&gun_module_left->module_links[0]);
    gun_module_left->module_links[1].link(&thruster_module_left->module_links[0]);
    gun_module_right->module_links[1].link(&thruster_module_right->module_links[0]);
    base_module->module_links[0].link(&laser_sensor_module->module_links[0]);

    // Add modules to Agent
    modules.push_back(std::move(base_module));
    modules.push_back(std::move(gun_module_right));
    modules.push_back(std::move(gun_module_left));
    modules.push_back(std::move(thruster_module_left));
    modules.push_back(std::move(thruster_module_right));
    modules.push_back(std::move(laser_sensor_module));

    // Sync agent with new modules
    update_body();
    register_actions();

    // If true, draws the hitboxes of each module on screen
    debug_draw = false;

    this->rng = rng;

    rigid_body->body->ApplyForce(b2Vec2(100, 100), b2Vec2(3, 3), true);
}

TestAgent::~TestAgent() {}

void TestAgent::act(std::vector<int> action_flags)
{
    for (const auto &module : modules)
    {
        module->update();
    }

    int current_position = 0;
    for (const auto &action : actions)
    {
        int next_position = current_position + action->flag_count;
        action->act(std::vector<int>(&action_flags[current_position], &action_flags[next_position]));
        current_position = next_position;
    }
}

std::vector<float> TestAgent::get_observation()
{
    std::vector<float> observation;
    for (const auto &module : modules)
    {
        std::vector<float> sensor_reading = module->get_sensor_reading();
        observation.insert(observation.end(), sensor_reading.begin(), sensor_reading.end());
    }
    return observation;
}

void TestAgent::begin_contact(RigidBody *other) {}
void TestAgent::end_contact(RigidBody *other) {}

RenderData TestAgent::get_render_data(bool lightweight)
{
    RenderData render_data;
    for (const auto &module : modules)
    {
        auto x = module->get_render_data(lightweight);
        render_data.append(x);
    }

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

void TestAgent::update_body()
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
        std::vector<b2PolygonShape> screen_shapes = module->shapes;
        for (auto &screen_shape : screen_shapes)
        {
            int vertex_count = screen_shape.GetVertexCount();
            b2Vec2 points[vertex_count];

            // Apply transform to all points in screen_shape
            for (int i = 0; i < vertex_count; ++i)
            {
                points[i] = b2Mul(module->transform, screen_shape.GetVertex(i));
            }
            screen_shape.Set(points, vertex_count);

            // Create the fixture
            b2FixtureDef fixture_def;
            fixture_def.shape = &screen_shape;
            fixture_def.density = 1;
            fixture_def.friction = 1;
            rigid_body->body->CreateFixture(&fixture_def);
        }
    }
}

void TestAgent::register_actions()
{
    actions = std::vector<IAction *>();
    for (const auto &module : modules)
    {
        for (const auto &action : module->actions)
        {
            actions.push_back(action.get());
        }
    }
}
}