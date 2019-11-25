#pragma once

#include <memory>
#include <string>

namespace SingularityTrainer
{
class IModule;
class Random;

class ModuleFactory
{
  private:
    Random &rng;

  public:
    ModuleFactory(Random &rng) : rng(rng) {}

    std::shared_ptr<IModule> create_module(const std::string &module_id);
};
}