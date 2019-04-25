#include <memory>
#include <vector>
#include <sstream>

#include <Box2D/Box2D.h>

#include "random.h"
#include "training/agents/test_agent.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/thruster_module.h"
#include "training/environments/ienvironment.h"
#include "training/rigid_body.h"
#include "graphics/colors.h"
#include "utilities.h"

namespace SingularityTrainer
{
TestAgent::TestAgent(b2World &world, Random *rng, IEnvironment &environment) : Agent(world, rng, environment)
{
    // Instantiate modules
    auto base_module = std::make_shared<BaseModule>();
    auto gun_module_right = std::make_shared<GunModule>();
    auto gun_module_left = std::make_shared<GunModule>();
    auto thruster_module_left = std::make_shared<ThrusterModule>();
    auto thruster_module_right = std::make_shared<ThrusterModule>();
    auto laser_sensor_module = std::make_shared<LaserSensorModule>();

    // Connect modules
    base_module->get_module_links()[1].link(&gun_module_right->get_module_links()[2]);
    base_module->get_module_links()[3].link(&gun_module_left->get_module_links()[0]);
    gun_module_left->get_module_links()[1].link(&thruster_module_left->get_module_links()[0]);
    gun_module_right->get_module_links()[1].link(&thruster_module_right->get_module_links()[0]);
    base_module->get_module_links()[0].link(&laser_sensor_module->get_module_links()[0]);

    // Add modules to Agent
    add_module(base_module);
    add_module(gun_module_right);
    add_module(gun_module_left);
    add_module(thruster_module_left);
    add_module(thruster_module_right);
    add_module(laser_sensor_module);

    // Sync agent with new modules
    update_body();
    register_actions();
}
}