#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

namespace SingularityTrainer
{
class CredentialsManager;
class IHttpClient;
class IO;
class ModuleTextureStore;
class ResourceManager;

class UnlockPartsWindow
{
  private:
    struct Part
    {
        std::string name;
        bool owned;
        long price;
    };

    std::atomic<bool> bought_part;
    CredentialsManager &credentials_manager;
    long credits;
    std::mutex credits_mutex;
    IHttpClient &http_client;
    IO &io;
    ModuleTextureStore &module_texture_store;
    std::vector<Part> parts;
    std::mutex parts_mutex;
    ResourceManager &resource_manager;
    Part *selected_part;
    std::atomic<bool> waiting_for_unlock_response;

  public:
    UnlockPartsWindow(CredentialsManager &credentials_manager,
                      IHttpClient &http_client,
                      IO &io,
                      ModuleTextureStore &module_texture_store,
                      ResourceManager &resource_manager);

    void unlock_part(const std::string &part, int timeout = 10);
    void refresh_info(int timeout = 10);
    bool update(bool &show);
};
}