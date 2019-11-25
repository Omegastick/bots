#pragma once

#include <string>
#include <vector>

namespace SingularityTrainer
{
class IO;
class ModuleTextureStore;
class ResourceManager;

class PartSelectorWindow
{
  private:
    IO &io;
    ModuleTextureStore &module_texture_store;
    ResourceManager &resource_manager;

  public:
    PartSelectorWindow(IO &io,
                       ModuleTextureStore &module_texture_store,
                       ResourceManager &resource_manager);

    std::string update(std::vector<std::string> &parts);
};
}