#pragma once

#include <memory>
#include <vector>

#include "training/bodies/body.h"

namespace ai
{
class Random;
class RigidBody;
class IEnvironment;

class TestBody : public Body
{
  public:
    TestBody(IModuleFactory &module_factory, Random &rng);
    TestBody(std::unique_ptr<RigidBody> rigid_body,
             IModuleFactory &module_factory,
             Random &rng,
             IEnvironment &environment);

    void setup();
};

class TestBodyFactory : public BodyFactory
{
  public:
    TestBodyFactory(IModuleFactory &module_factory, Random &rng)
        : BodyFactory(module_factory, rng) {}

    std::unique_ptr<Body> make(Random &rng)
    {
        return std::make_unique<TestBody>(module_factory, rng);
    }

    virtual std::unique_ptr<Body> make(b2World &world, Random &rng)
    {
        auto body = std::make_unique<TestBody>(module_factory, rng);
        auto rigid_body = std::make_unique<RigidBody>(
            b2_dynamicBody,
            b2Vec2_zero,
            world,
            nullptr,
            RigidBody::ParentTypes::Body);
        body->set_rigid_body(std::move(rigid_body));
        return body;
    }
};
}