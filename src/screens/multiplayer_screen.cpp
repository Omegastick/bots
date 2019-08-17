#include <memory>

#include <Box2D/Box2D.h>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <spdlog/spdlog.h>
#include <zmq_addon.hpp>

#include "multiplayer_screen.h"
#include "graphics/backend/shader.h"
#include "graphics/post_proc_layer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "networking/client_agent.h"
#include "networking/client_communicator.h"
#include "networking/messages.h"
#include "networking/msgpack_codec.h"
#include "screens/iscreen.h"
#include "third_party/zmq.hpp"
#include "training/agents/iagent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/body.h"
#include "training/environments/ienvironment.h"
#include "training/environments/playback_env.h"
#include "ui/multiplayer_screen/choose_agent_window.h"

namespace SingularityTrainer
{
MultiplayerScreen::MultiplayerScreen(double tick_length,
                                     std::unique_ptr<ChooseAgentWindow> choose_agent_window,
                                     IEnvironmentFactory &env_factory,
                                     IO &io,
                                     ResourceManager &resource_manager,
                                     Random &rng)
    : choose_agent_window(std::move(choose_agent_window)),
      env_factory(env_factory),
      io(io),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      resource_manager(resource_manager),
      rng(rng),
      server_address("tcp://localhost:7654"),
      state(MultiplayerScreen::State::ChooseAgent),
      tick_length(tick_length)
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);

    crt_post_proc_layer = std::make_unique<PostProcLayer>(resource_manager.shader_store.get("crt").get());
}

void MultiplayerScreen::update(double delta_time)
{
    if (state == MultiplayerScreen::State::ChooseAgent)
    {
        choose_agent();
    }
    else if (state == MultiplayerScreen::State::InputAddress)
    {
        input_address();
    }
    else if (state == MultiplayerScreen::State::WaitingToStart)
    {
        wait_for_start();
    }
    else if (state == MultiplayerScreen::State::Playing)
    {
        play(delta_time);
    }
}

void MultiplayerScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.push_post_proc_layer(crt_post_proc_layer.get());
    renderer.begin();

    if (env != nullptr)
    {
        renderer.scissor(-10, -20, 10, 20, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f));
        auto render_data = env->get_render_data(lightweight);
        renderer.draw(render_data,
                      glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f),
                      env->get_elapsed_time(),
                      lightweight);
    }

    auto crt_shader = resource_manager.shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
    crt_shader->set_uniform_1f("u_output_gamma", 1);
    crt_shader->set_uniform_1f("u_strength", 0.8);
    crt_shader->set_uniform_1f("u_distortion_factor", 0.1);

    renderer.end();
}

void MultiplayerScreen::choose_agent()
{
    agent = choose_agent_window->update();
    if (agent != nullptr)
    {
        state = MultiplayerScreen::State::InputAddress;
    }
}

void MultiplayerScreen::input_address()
{
    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    auto resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Always);
    ImGui::Begin("Multiplayer", NULL, ImGuiWindowFlags_NoResize);
    ImGui::InputText("Server Address", &server_address);
    if (ImGui::Button("Connect"))
    {
        // Connect to the server
        auto client_socket = std::make_unique<zmq::socket_t>(zmq_context, zmq::socket_type::dealer);
        std::string id(std::to_string(rng.next_int(0, 10000000)));
        client_socket->setsockopt(ZMQ_IDENTITY, id.c_str(), id.size());
        spdlog::info("Connecting to server");
        client_socket->connect(server_address);
        client_communicator = std::make_unique<ClientCommunicator>(std::move(client_socket));

        env = std::make_unique<PlaybackEnv>(env_factory.make(), tick_length);

        auto body_spec = env->get_bodies()[0]->to_json();

        ConnectMessage connect_message(body_spec.dump());
        auto encoded_connect_message = MsgPackCodec::encode(connect_message);
        spdlog::info("Sending connect message");
        client_communicator->send(encoded_connect_message);

        state = MultiplayerScreen::State::WaitingToStart;
    }
    ImGui::End();
}

void MultiplayerScreen::play(double delta_time)
{
    while (true)
    {
        std::string raw_message = client_communicator->get();
        if (raw_message.empty())
        {
            break;
        }

        auto message_object = MsgPackCodec::decode<msgpack::object_handle>(raw_message);

        auto type = get_message_type(message_object.get());
        if (type == MessageType::State)
        {
            spdlog::info("Received state");
            auto message = message_object->as<StateMessage>();

            auto action = client_agent->get_action(EnvState(message.agent_transforms,
                                                            message.entity_transforms,
                                                            message.tick));

            ActionMessage action_message(action, message.tick);
            auto encoded_action_message = MsgPackCodec::encode(action_message);
            client_communicator->send(encoded_action_message);

            env->add_new_state(EnvState(message.agent_transforms, message.entity_transforms, message.tick));
            env->add_events(std::move(message.events));
        }
    }

    env->update(delta_time);

    glm::vec2 resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.3f}, ImGuiCond_Once);
    ImGui::Begin("Health");
    auto bodies = env->get_bodies();
    for (const auto &body : bodies)
    {
        auto health = body->get_hp();
        double max_health = 10;
        ImGui::ProgressBar(health / max_health, {-1, 0}, fmt::format("{}/{}", health, max_health).c_str());
    }
    ImGui::End();

    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.5f}, ImGuiCond_Once);
    ImGui::Begin("Scores");
    for (const auto &score : env->get_scores())
    {
        ImGui::Text("%.1f", score);
    }
    ImGui::End();
}

void MultiplayerScreen::wait_for_start()
{
    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    auto resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Always);
    ImGui::Begin("Multiplayer", NULL, ImGuiWindowFlags_NoResize);
    ImGui::Text("Waiting for game to start...");
    ImGui::End();

    std::string raw_message = client_communicator->get();
    if (raw_message.empty())
    {
        return;
    }

    auto message_object = MsgPackCodec::decode<msgpack::object_handle>(raw_message);

    auto type = get_message_type(message_object.get());
    if (type == MessageType::ConnectConfirmation)
    {
        spdlog::info("Received connection confirmation");
        auto message = message_object->as<ConnectConfirmationMessage>();
        auto body_spec = env->get_bodies()[0]->to_json();
        client_agent = std::make_unique<ClientAgent>(std::move(agent), message.player_number, env_factory.make());
    }
    else if (type == MessageType::GameStart)
    {
        spdlog::info("Received game start message");
        auto message = message_object->as<GameStartMessage>();
        std::vector<nlohmann::json> body_specs;
        std::transform(message.body_specs.begin(), message.body_specs.end(),
                       std::back_inserter(body_specs),
                       [](const std::string &body_spec_string) {
                           return nlohmann::json::parse(body_spec_string);
                       });
        client_agent->set_bodies(body_specs);
        env->set_bodies(body_specs);

        state = MultiplayerScreen::State::Playing;
    }
}
}