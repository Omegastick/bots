#include <chrono>
#include <cstdlib>
#include <memory>
#include <iostream>
#include <thread>
#include <signal.h>
#include <stdlib.h>

#include <doctest.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "server_app.h"
#include "networking/messages.h"
#include "networking/msgpack_codec.h"
#include "third_party/httplib.h"
#include "training/environments/koth_env.h"

namespace SingularityTrainer
{
const std::string agones_url_base = "localhost";
volatile sig_atomic_t stop;

void inthand(int /*signum*/)
{
    stop = 1;
}

static void health_check()
{
    httplib::Client http_client(agones_url_base.c_str(), 59358);
    spdlog::info("Starting health checks");

    while (!stop)
    {
        auto response = http_client.Post("/health", "{}", "application/json");
        if (response == nullptr)
        {
            spdlog::error("Failed health check: No response");
        }
        else if (response->status != 200)
        {
            spdlog::error("Failed health check: {}", response->status);
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

ServerApp::ServerApp(std::unique_ptr<Game> game)
    : game(std::move(game)),
      http_client(agones_url_base.c_str(), 59358)
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");

    signal(SIGINT, inthand);
}

int ServerApp::run(int argc, char *argv[])
{
    argh::parser args(argv);
    // Tests
    if (args[{"-t", "--test"}])
    {
        return run_tests(argc, argv, args);
    }

    // Turn off logging
    if (args[{"-q", "--quiet"}])
    {
        spdlog::set_level(spdlog::level::off);
    }

    // Bind to ZeroMQ port
    int port;
    args({"-p", "--port"}, 7654) >> port;
    spdlog::info("Serving on port: {}", port);

    auto socket = std::make_unique<zmq::socket_t>(zmq_context, zmq::socket_type::router);
    socket->bind("tcp://*:" + std::to_string(port));
    server_communicator = std::make_unique<ServerCommunicator>(std::move(socket));

    // Signal to Agones that we are ready and start the health check thread
    std::thread health_thread;
    bool use_agones = args[{"--agones"}];
    if (use_agones)
    {
        spdlog::info("Marking server as ready");

        int retries = 0;
        while (true)
        {
            auto response = http_client.Post("/ready", "{}", "application/json");
            if (response != nullptr && response->status == 200)
            {
                break;
            }
            if (retries < 10)
            {
                retries += 1;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            else
            {
                throw std::runtime_error("Could not mark server as ready");
            }
        }

        health_thread = std::thread(health_check);

        wait_for_player_info();
    }

    GameStartMessage game_start_message;
    bool game_started = false;
    double start_time;

    // Main loop
    bool finished = false;
    while (!finished && !stop)
    {
        // Handle messages
        while (true)
        {
            MessageWithId raw_message = server_communicator->get();
            if (raw_message.id.empty())
            {
                break;
            }
            auto message_object = MsgPackCodec::decode<msgpack::object_handle>(
                raw_message.message);
            auto type = get_message_type(message_object.get());

            if (type == MessageType::Connect)
            {
                auto message = message_object->as<ConnectMessage>();
                auto json = nlohmann::json::parse(message.body_spec);
                if (use_agones)
                {
                    if (std::find(player_tokens.begin(), player_tokens.end(), message.token) ==
                        player_tokens.end())
                    {
                        spdlog::warn("User attempted to connect with bad token: {}",
                                     message.token);
                        continue;
                    }
                }
                players.push_back(raw_message.id);
                spdlog::info("{} connected", raw_message.id);
                game->add_body(json);
                game_start_message.body_specs.push_back(message.body_spec);
                ConnectConfirmationMessage reply(players.size() - 1);
                auto encoded_reply = MsgPackCodec::encode(reply);
                server_communicator->send(raw_message.id, encoded_reply);
            }
            else if (type == MessageType::Action)
            {
                auto message = message_object->as<ActionMessage>();
                game->set_action(message.tick,
                                 std::find(players.begin(),
                                           players.end(),
                                           raw_message.id) -
                                     players.begin(),
                                 message.actions);
            }
        }

        // If game is about to start, notify the clients
        if (!game_started && game_start_message.body_specs.size() == 2)
        {
            spdlog::info("Starting game");
            for (const auto &player : players)
            {
                auto encoded_game_start_message = MsgPackCodec::encode(game_start_message);
                server_communicator->send(player, encoded_game_start_message);
            }
            game_started = true;
            start_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            start_time *= 1e-9;
        }

        // Step environment
        double time_stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        time_stamp *= 1e-9;
        if (game_started && game->ready_to_tick(time_stamp))
        {
            auto tick_result = game->tick(time_stamp);
            if (tick_result.tick % 10 == 0)
            {
                auto fps = tick_result.tick / (time_stamp - start_time);
                spdlog::debug("FPS: {}", fps);
            }

            StateMessage reply(std::move(tick_result.agent_transforms),
                               std::move(tick_result.entity_transforms),
                               std::move(tick_result.events),
                               std::move(tick_result.hps),
                               std::move(tick_result.scores),
                               tick_result.done,
                               tick_result.tick);
            auto encoded_reply = MsgPackCodec::encode(reply);
            auto obj = MsgPackCodec::decode<msgpack::object_handle>(encoded_reply);
            for (const auto &player : players)
            {
                server_communicator->send(player, encoded_reply);
            }

            finished = tick_result.done;
            if (finished)
            {
                spdlog::info("Winner: {}", tick_result.victor);
                if (use_agones)
                {
                    update_elos(tick_result.victor);
                }
            }
        }
    }

    stop = true;
    if (health_thread.joinable())
    {
        health_thread.join();
    }

    // Shutdown
    if (use_agones)
    {
        // Signal to Agones that we are shutting down
        auto response = http_client.Post("/shutdown", "{}", "application/json");
        if (response == nullptr || response->status != 200)
        {
            throw std::runtime_error("Could not mark server as shut down");
        }
    }

    return 0;
}

int ServerApp::run_tests(int argc, char *argv[], const argh::parser &args)
{
    if (!args["--with-logs"])
    {
        spdlog::set_level(spdlog::level::warn);
    }
    doctest::Context context;

    context.setOption("order-by", "name");

    context.applyCommandLine(argc, argv);

    return context.run();
}

void ServerApp::update_elos(int victor)
{
    nlohmann::json json;
    json["players"] = player_usernames;
    json["result"] = victor;

    // Authorization token is retrieved from an environment variable
    // On Kubernetes this is stored as a secret
    std::string token = std::getenv("ST_CLOUD_TOKEN");
    httplib::Headers headers = {
        {"Authorization", "Bearer " + token}};

    std::string matchmaker_url = std::getenv("MATCHMAKER_URL");
    httplib::Client matchmaker(matchmaker_url.c_str());
    auto response = matchmaker.Post("/finish_game", headers, json.dump(), "application/json");

    if (response == nullptr)
    {
        throw std::runtime_error("Failed to mark game as finished: No response");
    }
    else if (response->status != 200)
    {
        std::string error = fmt::format("Failed to mark game as finished: {}", response->status);
        throw std::runtime_error(error);
    }

    // Check result is as expected: {success: true}
    auto response_json = nlohmann::json::parse(response->body);
    if (response_json != nlohmann::json{{"successs", true}})
    {
        std::string error = fmt::format("Failed to mark game as finished: {}", response->body);
        spdlog::error(error);
    }

    spdlog::info("Successfully matchmaker of result");
}

void ServerApp::wait_for_player_info()
{
    spdlog::info("Retrieving player information");
    auto response = http_client.Get(
        "/watch/gameserver",
        [&](const char *data, size_t data_length, size_t offset, uint64_t content_length) -> bool {
            spdlog::debug("{} - {} - {} - {}", data, data_length, offset, content_length);
            std::string raw_json(data, data_length);
            auto json = nlohmann::json::parse(raw_json);
            if (json["result"]["object_meta"]["annotations"].contains("player_1_username"))
            {
                auto annotations = json["result"]["object_meta"]["annotations"];
                player_usernames.push_back(annotations["player_1_username"]);
                player_usernames.push_back(annotations["player_2_username"]);
                player_tokens.push_back(annotations["player_1_token"]);
                player_tokens.push_back(annotations["player_2_token"]);
                return false;
            }
            return true;
        });

    while (player_tokens.size() < 2)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    spdlog::info("Players: {}", player_usernames);
    spdlog::info("Tokens: {}", player_tokens);
}
}