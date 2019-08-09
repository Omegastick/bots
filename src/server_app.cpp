#include <chrono>
#include <memory>
#include <iostream>
#include <signal.h>
#include <stdlib.h>

#include <doctest.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "server_app.h"
#include "networking/messages.h"
#include "networking/msgpack_codec.h"
#include "training/environments/koth_env.h"

namespace SingularityTrainer
{
volatile sig_atomic_t stop;

void inthand(int /*signum*/)
{
    stop = 1;
}

ServerApp::ServerApp(std::unique_ptr<Game> game)
    : game(std::move(game))
{
    // Logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("%^[%T %7l] %v%$");

    signal(SIGINT, inthand);
}

int ServerApp::run(int argc, char *argv[])
{
    argh::parser args(argv);
    if (args[{"-t", "--test"}])
    {
        return run_tests(argc, argv, args);
    }

    if (args[{"-q", "--quiet"}])
    {
        spdlog::set_level(spdlog::level::off);
    }

    int port;
    args({"-p", "--port"}, 7654) >> port;
    spdlog::info("Serving on port: {}", port);

    auto socket = std::make_unique<zmq::socket_t>(zmq_context, zmq::socket_type::router);
    socket->bind("tcp://*:" + std::to_string(port));
    server_communicator = std::make_unique<ServerCommunicator>(std::move(socket));

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