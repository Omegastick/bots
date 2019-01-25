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
    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, b2Vec2_zero, world, this, RigidBody::ParentTypes::Agent);

    std::unique_ptr<BaseModule> base_module = std::make_unique<BaseModule>(resource_manager, *rigid_body->body, this);
    std::unique_ptr<GunModule> gun_module_right = std::make_unique<GunModule>(resource_manager, *rigid_body->body, this);
    std::unique_ptr<GunModule> gun_module_left = std::make_unique<GunModule>(resource_manager, *rigid_body->body, this);

    base_module->module_links[1].link(&gun_module_right->module_links[0]);
    base_module->module_links[3].link(&gun_module_left->module_links[1]);

    modules.push_back(std::move(base_module));
    modules.push_back(std::move(gun_module_right));
    modules.push_back(std::move(gun_module_left));

    update_body();
    register_actions();

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
        // Copy the module's shape
        // It's important we leave the origina intact in case we need to do this again
        b2PolygonShape shape = module->shape;

        // Apply transform to all points in shape
        for (int i = 0; i < shape.m_count; ++i)
        {
            // Translate point
            shape.m_vertices[i] = module->transform.p + shape.m_vertices[i];
            // Rotate point
            shape.m_vertices[i] = rotate_point_around_point(shape.m_vertices[i], module->transform.q, shape.m_centroid);
        }

        // Create the fixture
        b2FixtureDef fixture_def;
        fixture_def.shape = &shape;
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