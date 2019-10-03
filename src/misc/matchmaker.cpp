#include <chrono>
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
const std::string find_game_url =
    "https://asia-northeast1-st-dev-252104.cloudfunctions.net/find_game";

Matchmaker::Matchmaker(CredentialsManager &credentials_manager, IHttpClient &http_client)
    : credentials_manager(credentials_manager),
      http_client(http_client) {}

std::future<std::string> Matchmaker::find_game(int timeout)
{
    std::string token = credentials_manager.get_token();
    if (token.empty())
    {
        throw std::runtime_error("Bearer token not set");
    }

    return std::async(
        std::launch::async,
        [=] {
            std::string gameserver_url;
            while (gameserver_url.empty())
            {
                auto response = http_client.post(find_game_url);
                auto future_status = response.wait_for(std::chrono::seconds(timeout));
                if (future_status == std::future_status::timeout)
                {
                    throw std::runtime_error("Find game request timed out");
                }
                auto json = response.get();
                if (json["status"] == "in_game")
                {
                    gameserver_url = json["gameserver"];
                }
            }
            return gameserver_url;
        });
}

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

        SUBCASE("Sets exception on future if connection times out")
        {
            std::promise<nlohmann::json> promise;
            ALLOW_CALL(http_client, post(_, _, _))
                .LR_RETURN(promise.get_future());

            auto url_future = matchmaker.find_game(0);

            promise.set_value(nlohmann::json{{"status", "in_game"},
                                             {"gameserver", "tcp://asd:123"}});

            CHECK_THROWS(url_future.get());
        }

        SUBCASE("Doesn't set future value until game is found")
        {
            std::promise<nlohmann::json> promise_1;
            ALLOW_CALL(http_client, post(_, _, _))
                .LR_RETURN(promise_1.get_future());
            std::promise<nlohmann::json> promise_2;
            ALLOW_CALL(http_client, post(_, _, _))
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
            ALLOW_CALL(http_client, post(_, _, _))
                .LR_RETURN(promise.get_future());

            auto url_future = matchmaker.find_game();

            promise.set_value(nlohmann::json{{"status", "in_game"},
                                             {"gameserver", "tcp://asd:123"}});
            CHECK(url_future.get() == "tcp://asd:123");
        }
    }
}
}