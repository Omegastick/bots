#pragma once

#include <future>
#include <string>

namespace ai
{
class CredentialsManager;
class IHttpClient;

class Matchmaker
{
  private:
    CredentialsManager &credentials_manager;
    IHttpClient &http_client;

    std::atomic<bool> stop;

  public:
    Matchmaker(CredentialsManager &credentials_manager, IHttpClient &http_client);

    void cancel();
    std::future<std::string> find_game(int timeout = 20, int frequency = 3);
};
}