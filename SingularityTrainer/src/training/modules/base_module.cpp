#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <glm/vec2.hpp>

#include "graphics/colors.h"
#include "resource_manager.h"
#include "training/agents/iagent.h"
#include "training/modules/base_module.h"
#include "training/modules/imodule.h"
#include "training/modules/module_link.h"

namespace SingularityTrainer
{
BaseModule::BaseModule(ResourceManager &resource_manager, b2Body &body, IAgent *agent)
{
    // Sprite
    sprite = std::make_unique<Sprite>("base_module");
    sprite->set_scale(glm::vec2(1, 1));
    sprite->set_origin(sprite->get_center());
    sprite->set_color(cl_white);

    // Box2D
    b2PolygonShape shape;
    shape.SetAsBox(0.5, 0.5);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, 0.5, 0, this));
    module_links.push_back(ModuleLink(0.5, 0, 90, this));
    module_links.push_back(ModuleLink(0, -0.5, 180, this));
    module_links.push_back(ModuleLink(-0.5, 0, 270, this));

    root = this;
    this->agent = agent;
}

BaseModule::~BaseModule() {}

std::vector<float> BaseModule::get_sensor_reading()
{
    b2Vec2 linear_velocity = agent->rigid_body->body->GetLinearVelocity();
    float angular_velocity = agent->rigid_body->body->GetAngularVelocity();
    return std::vector<float>{linear_velocity.x, linear_velocity.y, angular_velocity};
}
}
