#include <memory>
#include <vector>
#include <sstream>

#include <Box2D/Box2D.h>

#include "test_body.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "training/modules/base_module.h"
#include "training/modules/gun_module.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/thruster_module.h"
#include "training/environments/ienvironment.h"
#include "training/rigid_body.h"
#include "graphics/colors.h"
#include "misc/utilities.h"

namespace ai
{
TestBody::TestBody(std::unique_ptr<RigidBody> rigid_body,
                   IModuleFactory &module_factory,
                   Random &rng,
                   IEnvironment &environment) : Body(module_factory, rng)
{
    set_rigid_body(std::move(rigid_body));
    set_environment(environment);
    setup();
}

TestBody::TestBody(IModuleFactory &module_factory, Random &rng) : Body(module_factory, rng)
{
    setup();
}

void TestBody::setup()
{
    hp = 10;

    // Instantiate modules
    auto base_module = module_factory.make("base_module");
    auto gun_module_right = module_factory.make("gun_module");
    auto gun_module_left = module_factory.make("gun_module");
    auto thruster_module_left = module_factory.make("thruster_module");
    auto thruster_module_right = module_factory.make("thruster_module");
    auto laser_sensor_module = module_factory.make("laser_sensor_module");

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
