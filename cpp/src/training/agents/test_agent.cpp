#include <vector>

#include "training/agents/test_agent.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
TestAgent::TestAgent() {}
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