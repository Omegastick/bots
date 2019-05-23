#pragma once

#include <string>
#include <vector>

namespace SingularityTrainer
{
class IO;
class ResourceManager;

class PartSelectorWindow
{
  private:
    IO *io;
    ResourceManager *resource_manager;

  public:
    PartSelectorWindow(IO &io, ResourceManager &resource_manager);

    std::string update(std::vector<std::string> &parts);
};
}