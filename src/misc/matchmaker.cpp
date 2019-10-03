#include <future>
#include <string>

#include <doctest.h>
#include <nlohmann/json.hpp>
#include <trompeloeil.hpp>

#include "matchmaker.h"
#include "misc/credentials_manager.h"
#include "misc/ihttp_client.h"

namespace SingularityTrainer
{
Matchmaker::Matchmaker(CredentialsManager &credentials_manager, IHttpClient &http_client)
    : credentials_manager(credentials_manager),
      http_client(http_client) {}

std::future<std::string> Matchmaker::find_game() {}

using trompeloeil::_;

TEST_CASE("Matchmaker")
{
    MockHttpClient http_client;
    CredentialsManager credentials_manager(http_client);
    credentials_manager.set_token("test_token");
    Matchmaker matchmaker(credentials_manager, http_client);

    SUBCASE("find_game()")
    {
        SUBCASE("Throws if no token is stored")
        {
            credentials_manager.set_token("");
            CHECK_THROWS(matchmaker.find_game());
        }

        SUBCASE("Doesn't set future value until game is found")
        {
            std::promise<nlohmann::json> promise_1;
            ALLOW_CALL(http_client, post(_, _))
                .LR_RETURN(promise_1.get_future());
            std::promise<nlohmann::json> promise_2;
            ALLOW_CALL(http_client, post(_, _))
                .LR_RETURN(promise_2.get_future());

            auto url_future = matchmaker.find_game();

            CHECK(url_future.wait_for(std::chrono::seconds(0)) == std::future_status::timeout);

            promise_1.set_value(nlohmann::json{{"status", "waiting_for_game"}});
            CHECK(url_future.wait_for(std::chrono::seconds(0)) == std::future_status::timeout);

            promise_2.set_value(nlohmann::json{{"status", "in_game"},
                                               {"gameserver", "tcp://asd:123"}});
            CHECK(url_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
        }

        SUBCASE("Sets future value to gameserver URL")
        {
            std::promise<nlohmann::json> promise;
            ALLOW_CALL(http_client, post(_, _))
                .LR_RETURN(promise.get_future());

            auto url_future = matchmaker.find_game();

            CHECK(url_future.wait_for(std::chrono::seconds(0)) == std::future_status::timeout);

            promise.set_value(nlohmann::json{{"status", "in_game"},
                                             {"gameserver", "tcp://asd:123"}});
            CHECK(url_future.get() == "tcp://asd:123");
        }
    }
}
}