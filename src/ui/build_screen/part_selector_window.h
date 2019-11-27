#pragma once

#include <string>
#include <vector>

namespace SingularityTrainer
{
class CredentialsManager;
class IHttpClient;
class IO;
class ModuleTextureStore;
class ResourceManager;

class PartSelectorWindow
{
  private:
    CredentialsManager &credentials_manager;
    IHttpClient &http_client;
    IO &io;
    ModuleTextureStore &module_texture_store;
    std::vector<std::string> parts;
    ResourceManager &resource_manager;

  public:
    PartSelectorWindow(CredentialsManager &credentials_manager,
                       IHttpClient &http_client,
                       IO &io,
                       ModuleTextureStore &module_texture_store,
                       ResourceManager &resource_manager);

    void refresh_parts(int timeout = 10);
    std::string update();
};
}