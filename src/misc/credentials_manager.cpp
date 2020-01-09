#include <string>

#include <doctest.h>
#include <nlohmann/json.hpp>
#include <trompeloeil.hpp>

#include "credentials_manager.h"
#include "misc/ihttp_client.h"

namespace ai
{
CredentialsManager::CredentialsManager(IHttpClient &http_client)
    : http_client(http_client)
{
}

void CredentialsManager::login(const std::string &username)
{
    nlohmann::json json = {{"username", username}};
    auto response = http_client.post(st_cloud_base_url + "login", {{"username", username}});
    token = response.get()["token"];
    this->username = username;
}

using trompeloeil::_;

TEST_CASE("CredentialsManager")
{
    MockHttpClient http_client;
    CredentialsManager credentials_manager(http_client);

    SUBCASE("login()")
    {
        SUBCASE("Sets token")
        {
            std::promise<nlohmann::json> promise;
            promise.set_value(nlohmann::json{{"token", "asd123"}});
            ALLOW_CALL(http_client, post(_, _, _))
                .LR_RETURN(promise.get_future());

            credentials_manager.login("asd");

            DOCTEST_CHECK(credentials_manager.get_token() == "asd123");
        }

        SUBCASE("Sets username")
        {
            std::promise<nlohmann::json> promise;
            promise.set_value(nlohmann::json{{"token", "asd123"}});
            ALLOW_CALL(http_client, post(_, _, _))
                .LR_RETURN(promise.get_future());

            credentials_manager.login("asd");

            DOCTEST_CHECK(credentials_manager.get_username() == "asd");
        }
    }
}
}