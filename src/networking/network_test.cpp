#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <fmt/ostream.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "server_app.h"
#include "audio/audio_engine.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "networking/client_communicator.h"
#include "networking/client_agent.h"
#include "networking/game.h"
#include "networking/messages.h"
#include "networking/msgpack_codec.h"
#include "third_party/di.hpp"
#include "third_party/httplib.h"
#include "third_party/zmq.hpp"
#include "training/agents/random_agent.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"
#include "training/environments/koth_env.h"

namespace di = boost::di;

namespace ai
{

void run_client(const std::string &id, const std::string &token)
{
    zmq::context_t zmq_context;

    auto client_socket = std::make_unique<zmq::socket_t>(zmq_context, zmq::socket_type::dealer);
    client_socket->setsockopt(ZMQ_IDENTITY, id.c_str(), id.size());
    client_socket->connect("tcp://localhost:7654");
    ClientCommunicator client_communicator(std::move(client_socket));

    Random rng(0);
    MockAudioEngine audio_engine;
    BulletFactory bullet_factory(audio_engine);
    ModuleFactory module_factory(bullet_factory, rng);
    TestBodyFactory body_factory(module_factory, rng);
    KothEnvFactory env_factory(100, body_factory, bullet_factory);
    auto env = env_factory.make();

    auto body_spec = env->get_bodies()[0]->to_json();

    auto agent = std::make_unique<RandomAgent>(body_spec, rng, "Random agent");

    std::unique_ptr<ClientAgent> client_agent;

    ConnectMessage connect_message(body_spec.dump(), token);
    auto encoded_connect_message = MsgPackCodec::encode(connect_message);
    client_communicator.send(encoded_connect_message);

    bool finished = false;
    while (!finished)
    {
        std::string raw_message = client_communicator.get();
        if (raw_message.empty())
        {
            continue;
        }

        auto message_object = MsgPackCodec::decode<msgpack::object_handle>(raw_message);

        auto type = get_message_type(message_object.get());
        if (type == MessageType::ConnectConfirmation)
        {
            auto message = message_object->as<ConnectConfirmationMessage>();
            client_agent = std::make_unique<ClientAgent>(std::move(agent), message.player_number, std::move(env));
        }
        else if (type == MessageType::GameStart)
        {
            auto message = message_object->as<GameStartMessage>();
            std::vector<nlohmann::json> body_specs;
            std::transform(message.body_specs.begin(), message.body_specs.end(),
                           std::back_inserter(body_specs),
                           [](const std::string &body_spec_string) {
                               return nlohmann::json::parse(body_spec_string);
                           });
            client_agent->set_bodies(body_specs);
        }
        else if (type == MessageType::State)
        {
            spdlog::debug("Received state message: {}", message_object.get());
            auto message = message_object->as<StateMessage>();
            finished = message.done;

            auto action = client_agent->get_action(EnvState(message.agent_transforms,
                                                            message.entity_transforms,
                                                            message.hps,
                                                            message.scores,
                                                            message.tick));

            ActionMessage action_message(action, message.tick);
            auto encoded_action_message = MsgPackCodec::encode(action_message);
            client_communicator.send(encoded_action_message);
        }
    }
}

TEST_CASE("Network")
{
    const auto injector = di::make_injector(
        di::bind<int>.named(MaxSteps).to(100),
        di::bind<double>.named(TickLength).to(0.001),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
        di::bind<IAudioEngine>.to<AudioEngine>(),
        di::bind<IModuleFactory>.to<ModuleFactory>(),
        di::bind<IBulletFactory>.to<BulletFactory>());
    auto app = injector.create<ServerApp>();
    char filepath[] = "./asd";
    char quiet[] = "--quiet";
    char *argv[3] = {filepath, quiet, NULL};
    auto server_thread = std::thread([&] { app.run(2, argv); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    nlohmann::json json;
    json["players"] = nlohmann::json::array({"Zero", "One"});
    json["tokens"] = nlohmann::json::array({"asd", "sdf"});

    auto client_0_thread = std::thread([&] { run_client("One", "asd"); });
    auto client_1_thread = std::thread([&] { run_client("Two", "sdf"); });

    client_0_thread.join();
    client_1_thread.join();
    server_thread.join();
}
}