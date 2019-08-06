#include <chrono>
#include <memory>
#include <iostream>
#include <signal.h>
#include <stdlib.h>

#include <doctest.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "server_app.h"
#include "server/messages.h"
#include "server/msgpack_codec.h"
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

    int port;
    args({"-p", "--port"}, 7654) >> port;
    spdlog::info("Serving on port: {}", port);

    auto socket = std::make_unique<zmq::socket_t>(zmq_context, zmq::socket_type::router);
    socket->bind("tcp://*:" + port);
    server_communicator = std::make_unique<ServerCommunicator>(std::move(socket));

    // Main loop
    bool finished = false;
    while (!finished)
    {
        // Handle messages
        MessageWithId raw_message{};
        while (raw_message.id.empty())
        {
            raw_message = server_communicator->get();
            auto message_object = MsgPackCodec::decode<msgpack::object_handle>(raw_message.message);
            auto type = get_message_type(message_object.get());

            if (type == MessageType::Connect)
            {
                auto message = message_object->as<ConnectMessage>();
                auto json = nlohmann::json::parse(message.body_spec);
                players.push_back(raw_message.id);
                game->add_body(json);
                ConnectConfirmationMessage reply(players.size());
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

        // Step environment
        double time_stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count() * 1e-9;
        if (game->ready_to_tick(time_stamp))
        {
            auto tick_result = game->tick();
            StateMessage reply(tick_result.agent_transforms,
                               tick_result.entity_transforms,
                               tick_result.tick);
            auto encoded_reply = MsgPackCodec::encode(reply);
            for (const auto &player : players)
            {
                server_communicator->send(player, encoded_reply);
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