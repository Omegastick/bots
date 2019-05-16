#pragma once

#include <string>
#include <vector>

namespace SingularityTrainer
{
class ResourceManager;

class PartSelectorWindow
{
  private:
    ResourceManager *resource_manager;

  public:
    PartSelectorWindow(ResourceManager &resource_manager);

    std::string update(std::vector<std::string> &parts);
};
}