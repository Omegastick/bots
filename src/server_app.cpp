#define CPPHTTPLIB_THREAD_POOL_COUNT 0

#include <chrono>
#include <memory>
#include <iostream>
#include <signal.h>
#include <stdlib.h>

#include <doctest.h>
#include <fmt/format.h>
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

    while (!stop)
    {
        auto response = http_client.Post("/health", "{}", "application/json");
        if (response != nullptr && response->status == 200)
        {
            spdlog::info("Health ping sent");
        }
        else
        {
            if (response == nullptr)
            {
                spdlog::error("Failed health check: No response");
            }
            else
            {
                spdlog::error("Failed health check: {}", response->status);
            }
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
            else
            {
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
        }

        health_thread = std::thread(health_check);
    }

    GameStartMessage game_start_message;
    bool game_started = false;

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
            auto message_object = MsgPackCodec::decode<msgpack::object_handle>(raw_message.message);
            auto type = get_message_type(message_object.get());

            if (type == MessageType::Connect)
            {
                auto message = message_object->as<ConnectMessage>();
                auto json = nlohmann::json::parse(message.body_spec);
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
                                 std::find(players.begin(), players.end(), raw_message.id) - players.begin(),
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
        }

        // Step environment
        double time_stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count() * 1e-9;
        if (game_started && game->ready_to_tick(time_stamp))
        {
            auto tick_result = game->tick(time_stamp);
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
            }
        }
    }

    stop = true;
    health_thread.join();

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
        spdlog::set_level(spdlog::level::off);
    }
    doctest::Context context;

    context.setOption("order-by", "name");

    context.applyCommandLine(argc, argv);

    return context.run();
}
}