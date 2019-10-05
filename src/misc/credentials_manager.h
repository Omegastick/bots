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
    std::string username;

  public:
    CredentialsManager(IHttpClient &http_client);

    void login(const std::string &username);

    const std::string &get_token() const { return token; }
    const std::string &get_username() const { return username; }
    void set_token(const std::string &token) { this->token = token; }
    void set_username(const std::string &username) { this->username = username; }
};
}