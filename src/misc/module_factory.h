#pragma once

#include <memory>
#include <string>

namespace SingularityTrainer
{
class IModule;

class ModuleFactory
{
  private:
    ModuleFactory() {}

  public:
    static std::shared_ptr<IModule> create_module(std::string &module_id);
};
}