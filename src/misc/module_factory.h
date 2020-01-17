#pragma once

#include <memory>
#include <string>

#include <trompeloeil.hpp>

namespace ai
{
class IBulletFactory;
class IModule;
class Random;

class IModuleFactory
{
  public:
    virtual std::shared_ptr<IModule> make(const std::string &module_id) = 0;
};

class ModuleFactory : public IModuleFactory
{
  private:
    IBulletFactory &bullet_factory;
    Random &rng;

  public:
    ModuleFactory(IBulletFactory &bullet_factory, Random &rng)
        : bullet_factory(bullet_factory), rng(rng) {}

    std::shared_ptr<IModule> make(const std::string &module_id) override;
};

class MockModuleFactory : public trompeloeil::mock_interface<IModuleFactory>
{
  public:
    IMPLEMENT_MOCK1(make);
};
}