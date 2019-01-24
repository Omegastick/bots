#include <Box2D/Box2D.h>
#include <memory>
#include <vector>

#include "training/agents/test_agent.h"
#include "training/modules/base_module.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
TestAgent::TestAgent(ResourceManager &resource_manager, b2World &world)
{
    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, b2Vec2_zero, world, this, RigidBody::ParentTypes::Agent);

    modules.push_back(std::make_unique<BaseModule>(resource_manager, *rigid_body->body));
}
TestAgent::~TestAgent() {}

void TestAgent::act(std::vector<int> actions) {}
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
}