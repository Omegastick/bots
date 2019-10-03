#pragma once

#include <string>

namespace SingularityTrainer
{
class IHttpClient;

class CredentialsManager
{
  private:
    IHttpClient &http_client;
    std::string token;

  public:
    CredentialsManager(IHttpClient &http_client);

    void login(const std::string &username);

    const std::string &get_token() const { return token; }
    void set_token(const std::string &token) { this->token = token; }
};
}