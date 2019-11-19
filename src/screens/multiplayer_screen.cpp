#include <chrono>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>
#include <fmt/format.h>
#include <doctest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <spdlog/spdlog.h>

#include "multiplayer_screen.h"
#include "graphics/backend/shader.h"
#include "graphics/post_processing/post_proc_layer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "misc/credentials_manager.h"
#include "misc/io.h"
#include "misc/matchmaker.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "networking/client_agent.h"
#include "networking/client_communicator.h"
#include "networking/messages.h"
#include "networking/msgpack_codec.h"
#include "screens/iscreen.h"
#include "third_party/zmq.hpp"
#include "third_party/zmq_addon.hpp"
#include "training/agents/iagent.h"
#include "training/agents/random_agent.h"
#include "training/bodies/body.h"
#include "training/environments/ienvironment.h"
#include "training/environments/playback_env.h"
#include "ui/back_button.h"
#include "ui/multiplayer_screen/choose_agent_window.h"

namespace SingularityTrainer
{
MultiplayerScreen::MultiplayerScreen(double tick_length,
                                     std::unique_ptr<ChooseAgentWindow> choose_agent_window,
                                     CredentialsManager &credentials_manager,
                                     IEnvironmentFactory &env_factory,
                                     IO &io,
                                     Matchmaker &matchmaker,
                                     ResourceManager &resource_manager,
                                     Random &rng,
                                     ScreenManager &screen_manager)
    : choose_agent_window(std::move(choose_agent_window)),
      credentials_manager(credentials_manager),
      done_tick(-1),
      env_factory(env_factory),
      io(io),
      matchmaker(matchmaker),
      resource_manager(resource_manager),
      rng(rng),
      screen_manager(screen_manager),
      server_address("tcp://localhost:7654"),
      should_clear_particles(true),
      state(MultiplayerScreen::State::ChooseAgent),
      tick_length(tick_length)
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("square_hull", "images/square_hull.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_shader("distortion",
                                 "shaders/distortion.vert",
                                 "shaders/distortion.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);
    resource_manager.load_font("roboto-32", "fonts/Roboto-Regular.ttf", 32);

    auto resolution = io.get_resolution();
    distortion_layer = std::make_unique<DistortionLayer>(resource_manager,
                                                         resolution.x,
                                                         resolution.y);

    zmq_context.setctxopt(ZMQ_BLOCKY, false);
}

MultiplayerScreen::~MultiplayerScreen()
{
    matchmaker.cancel();
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
    else if (state == MultiplayerScreen::State::WaitingForMatchmaker)
    {
        wait_for_matchmaker();
    }
    else if (state == MultiplayerScreen::State::WaitingToStart)
    {
        wait_for_start();
    }
    else if (state == MultiplayerScreen::State::Playing)
    {
        play(delta_time);
    }
    else if (state == MultiplayerScreen::State::ConnectionFailure)
    {
        connection_failure();
    }

    auto resolution = io.get_resolutionf();
    back_button(screen_manager, resolution);
}

void MultiplayerScreen::draw(Renderer &renderer, bool lightweight)
{
    const double view_height = 50;
    auto view_top = view_height * 0.5;
    glm::vec2 resolution = io.get_resolutionf();
    auto view_right = view_top * (resolution.x / resolution.y);
    auto projection = glm::ortho(-view_right, view_right, -view_top, view_top);
    renderer.set_view(projection);
    renderer.set_distortion_layer(*distortion_layer);

    if (should_clear_particles)
    {
        renderer.clear_particles();
        should_clear_particles = false;
    }

    if (env != nullptr)
    {
        renderer.scissor(-10, -20, 10, 20, projection);
        env->draw(renderer, lightweight);

        if (state == MultiplayerScreen::State::Finished)
        {
            Text winner_text;
            if (winner == player_number)
            {
                winner_text.text = "You win";
            }
            else if (winner == -1)
            {
                winner_text.text = "Draw";
            }
            else
            {
                winner_text.text = "You lost";
            }
            winner_text.font = "roboto-16";
            winner_text.transform.set_position({0, 0});
            winner_text.transform.set_scale({0.3, 0.3});
            const double character_width = 2.2;
            double width = character_width * winner_text.text.size();
            const double height = 2.4;
            winner_text.transform.set_origin({width / 2., height / 2.});
            renderer.draw(winner_text);
        }
    }
}

void MultiplayerScreen::choose_agent()
{
    agent = choose_agent_window->update();
    if (agent != nullptr && io.get_key_pressed(GLFW_KEY_LEFT_CONTROL))
    {
        state = MultiplayerScreen::State::InputAddress;
    }
    else if (agent != nullptr)
    {
        server_address_future = matchmaker.find_game();
        state = MultiplayerScreen::State::WaitingForMatchmaker;
    }
}

void MultiplayerScreen::connect()
{
    spdlog::debug("Connecting to {}", server_address);
    auto client_socket = std::make_unique<zmq::socket_t>(zmq_context, zmq::socket_type::dealer);
    client_socket->setsockopt(ZMQ_LINGER, 0);
    std::string id(std::to_string(rng.next_int(0, 10000000)));
    client_socket->setsockopt(ZMQ_IDENTITY, id.c_str(), id.size());
    spdlog::info("Connecting to server");
    client_socket->connect(server_address);
    client_communicator = std::make_unique<ClientCommunicator>(std::move(client_socket));

    env = std::make_unique<PlaybackEnv>(env_factory.make(), tick_length);

    ConnectMessage connect_message(agent->get_body_spec().dump(),
                                   credentials_manager.get_token());
    auto encoded_connect_message = MsgPackCodec::encode(connect_message);
    spdlog::info("Sending connect message");
    client_communicator->send(encoded_connect_message);

    state = MultiplayerScreen::State::WaitingToStart;
}

void MultiplayerScreen::input_address()
{
    const auto &imgui_io = ImGui::GetIO();
    ImGui::SetNextWindowPos({imgui_io.DisplaySize.x * 0.5f, imgui_io.DisplaySize.y * 0.5f},
                            ImGuiCond_Always,
                            {0.5, 0.5f});
    auto resolution = io.get_resolutionf();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Always);
    ImGui::Begin("Multiplayer", NULL, ImGuiWindowFlags_NoResize);
    ImGui::InputText("Server Address", &server_address);
    if (ImGui::Button("Connect"))
    {
        connect();
    }
    ImGui::End();
}

void MultiplayerScreen::connection_failure()
{
    const auto &imgui_io = ImGui::GetIO();
    ImGui::SetNextWindowPos({imgui_io.DisplaySize.x * 0.5f, imgui_io.DisplaySize.y * 0.5f},
                            ImGuiCond_Always,
                            {0.5, 0.5f});
    auto resolution = io.get_resolutionf();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Always);
    ImGui::Begin("Multiplayer", NULL, ImGuiWindowFlags_NoResize);
    ImGui::Text("Couldn't connect to the server");
    if (ImGui::Button("Ok"))
    {
        screen_manager.close_screen();
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
        if (type != MessageType::State)
        {
            continue;
        }
        auto message = message_object->as<StateMessage>();

        auto action = client_agent->get_action(EnvState(message.agent_transforms,
                                                        message.entity_transforms,
                                                        message.hps,
                                                        message.scores,
                                                        message.tick));

        ActionMessage action_message(action, message.tick);
        auto encoded_action_message = MsgPackCodec::encode(action_message);
        client_communicator->send(encoded_action_message);

        env->add_new_state(EnvState(message.agent_transforms,
                                    message.entity_transforms,
                                    message.hps,
                                    message.scores,
                                    message.tick));
        env->add_events(std::move(message.events));

        if (!message.done)
        {
            continue;
        }

        if (message.scores[0] > message.scores[1])
        {
            winner = 0;
        }
        else if (message.scores[1] > message.scores[0])
        {
            winner = 1;
        }
        else
        {
            winner = -1;
        }
        done_tick = message.tick;
    }

    if (done_tick >= 0 && env->get_elapsed_time() * 10 >= done_tick)
    {
        state = MultiplayerScreen::State::Finished;
    }

    env->update(delta_time);

    glm::vec2 resolution = io.get_resolution();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Once);
    ImGui::SetNextWindowPos({resolution.x * 0.05f, resolution.y * 0.3f}, ImGuiCond_Once);
    ImGui::Begin("Health");
    auto bodies = env->get_bodies();
    for (const auto &body : bodies)
    {
        float health = body->get_hp();
        float max_health = 10;
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

void MultiplayerScreen::wait_for_matchmaker()
{
    const auto &imgui_io = ImGui::GetIO();
    ImGui::SetNextWindowPos({imgui_io.DisplaySize.x * 0.5f, imgui_io.DisplaySize.y * 0.5f},
                            ImGuiCond_Always,
                            {0.5, 0.5f});
    auto resolution = io.get_resolutionf();
    ImGui::SetNextWindowSize({resolution.x * 0.2f, resolution.y * 0.1f}, ImGuiCond_Always);
    ImGui::Begin("Multiplayer", NULL, ImGuiWindowFlags_NoResize);
    ImGui::Text("Searching for a game...");
    ImGui::End();

    auto future_status = server_address_future.wait_for(std::chrono::seconds(0));
    if (future_status == std::future_status::timeout)
    {
        return;
    }

    try
    {
        server_address = server_address_future.get();
        connect();
    }
    catch (std::exception &exception)
    {
        spdlog::error(exception.what());
        state = MultiplayerScreen::State::ConnectionFailure;
    }
}

void MultiplayerScreen::wait_for_start()
{
    const auto &imgui_io = ImGui::GetIO();
    ImGui::SetNextWindowPos({imgui_io.DisplaySize.x * 0.5f, imgui_io.DisplaySize.y * 0.5f},
                            ImGuiCond_Always,
                            {0.5, 0.5f});
    auto resolution = io.get_resolutionf();
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
        player_number = message.player_number;
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