#include <memory>
#include <vector>
#include <sstream>

#include <Box2D/Box2D.h>

#include "misc/random.h"
#include "training/bodies/test_body.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/thruster_module.h"
#include "training/environments/ienvironment.h"
#include "training/rigid_body.h"
#include "graphics/colors.h"
#include "misc/utilities.h"

namespace SingularityTrainer
{
TestBody::TestBody(std::unique_ptr<RigidBody> rigid_body,
                   Random &rng,
                   IEnvironment &environment) : Body(rng)
{
    set_rigid_body(std::move(rigid_body));
    set_environment(environment);
    setup();
}

TestBody::TestBody(Random &rng) : Body(rng)
{
    setup();
}

void TestBody::setup()
{
    // Instantiate modules
    auto base_module = std::make_shared<BaseModule>();
    auto gun_module_right = std::make_shared<GunModule>();
    auto gun_module_left = std::make_shared<GunModule>();
    auto thruster_module_left = std::make_shared<ThrusterModule>();
    auto thruster_module_right = std::make_shared<ThrusterModule>();
    auto laser_sensor_module = std::make_shared<LaserSensorModule>(20, 180, 20);

    // Connect modules
    base_module->get_module_links()[1].link(gun_module_right->get_module_links()[2]);
    base_module->get_module_links()[3].link(gun_module_left->get_module_links()[0]);
    gun_module_left->get_module_links()[1].link(thruster_module_left->get_module_links()[0]);
    gun_module_right->get_module_links()[1].link(thruster_module_right->get_module_links()[0]);
    base_module->get_module_links()[0].link(laser_sensor_module->get_module_links()[0]);

    // Add modules to Body
    add_module(base_module);
    add_module(gun_module_right);
    add_module(gun_module_left);
    add_module(thruster_module_left);
    add_module(thruster_module_right);
    add_module(laser_sensor_module);
}
}
