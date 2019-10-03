#pragma once

#include <future>
#include <string>

namespace SingularityTrainer
{
class CredentialsManager;
class IHttpClient;

class Matchmaker
{
  private:
    CredentialsManager &credentials_manager;
    IHttpClient &http_client;

  public:
    Matchmaker(CredentialsManager &credentials_manager, IHttpClient &http_client);

    std::future<std::string> find_game(int timeout = 5);
};
}