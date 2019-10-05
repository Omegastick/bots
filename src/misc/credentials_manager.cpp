#include <string>

#include <doctest.h>
#include <nlohmann/json.hpp>
#include <trompeloeil.hpp>

#include "credentials_manager.h"
#include "misc/ihttp_client.h"

namespace SingularityTrainer
{
const std::string base_url = "https://asia-northeast1-st-dev-252104.cloudfunctions.net/";

CredentialsManager::CredentialsManager(IHttpClient &http_client)
    : http_client(http_client)
{
}

void CredentialsManager::login(const std::string &username)
{
    nlohmann::json json = {{"username", username}};
    auto response = http_client.post(base_url + "login", {{"username", username}});
    token = response.get()["token"];
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

            CHECK(credentials_manager.get_token() == "asd123");
        }
    }
}
}