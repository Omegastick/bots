#include <Box2D/Box2D.h>
#include <memory>
#include <vector>

#include "training/agents/test_agent.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/rigid_body.h"
#include "utilities.h"

namespace SingularityTrainer
{
TestAgent::TestAgent(ResourceManager &resource_manager, b2World &world)
{
    // Rigid body
    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, b2Vec2_zero, world, this, RigidBody::ParentTypes::Agent);

    // Instantiate modules
    std::unique_ptr<BaseModule> base_module = std::make_unique<BaseModule>(resource_manager, *rigid_body->body, this);
    std::unique_ptr<GunModule> gun_module_right = std::make_unique<GunModule>(resource_manager, *rigid_body->body, this);
    std::unique_ptr<GunModule> gun_module_left = std::make_unique<GunModule>(resource_manager, *rigid_body->body, this);

    // Connect modules
    base_module->module_links[1].link(&gun_module_right->module_links[0]);
    base_module->module_links[3].link(&gun_module_left->module_links[1]);

    // Add modules to Agent
    modules.push_back(std::move(base_module));
    modules.push_back(std::move(gun_module_right));
    modules.push_back(std::move(gun_module_left));

    // Sync agent with new modules
    update_body();
    register_actions();

    // If true, draws the hitboxes of each module on screen
    debug_draw = true;

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
        current_position += next_position;
    }
}

std::vector<float> TestAgent::get_observation() { return std::vector<float>(); }
void TestAgent::begin_contact(RigidBody *other) {}
void TestAgent::end_contact(RigidBody *other) {}

void TestAgent::draw(sf::RenderTarget &render_target)
{
    for (const auto &module : modules)
    {
        module->draw(render_target);
    }

    if (debug_draw)
    {
        b2Transform transform = rigid_body->body->GetTransform();
        for (b2Fixture *fixture = rigid_body->body->GetFixtureList(); fixture; fixture = fixture->GetNext())
        {
            b2PolygonShape *b2_shape = (b2PolygonShape *)fixture->GetShape();
            sf::ConvexShape screen_shape(b2_shape->GetVertexCount());
            screen_shape.setOutlineColor(sf::Color::Green);
            screen_shape.setOutlineThickness(0.02);
            screen_shape.setFillColor(sf::Color::Transparent);
            for (int i = 0; i < b2_shape->m_count; ++i)
            {
                b2Vec2 vertex_position = b2Mul(transform, b2_shape->GetVertex(i));
                screen_shape.setPoint(i, sf::Vector2f(vertex_position.x, vertex_position.y));
            }
            render_target.draw(screen_shape);
        }
    }
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
        // It's important we leave the origina intact in case we need to do this again
        b2PolygonShape screen_shape = module->shape;

        // Apply transform to all points in screen_shape
        for (int i = 0; i < screen_shape.m_count; ++i)
        {
            // Translate point
            screen_shape.m_vertices[i] = module->transform.p + screen_shape.m_vertices[i];
            // Rotate point
            screen_shape.m_vertices[i] = rotate_point_around_point(screen_shape.m_vertices[i], module->transform.q, screen_shape.m_centroid);
        }

        // Create the fixture
        b2FixtureDef fixture_def;
        fixture_def.shape = &screen_shape;
        fixture_def.density = 1;
        fixture_def.friction = 1;
        rigid_body->body->CreateFixture(&fixture_def);
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