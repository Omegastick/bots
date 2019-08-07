#include <memory>

#include <Box2D/Box2D.h>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

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
#include "training/agents/iagent.h"
#include "training/bodies/body.h"
#include "training/environments/ienvironment.h"
#include "training/environments/playback_env.h"

namespace SingularityTrainer
{
MultiplayerScreen::MultiplayerScreen(std::unique_ptr<ClientAgent> client_agent,
                                     std::unique_ptr<ClientCommunicator> client_communicator,
                                     std::unique_ptr<PlaybackEnv> env,
                                     IO &io,
                                     ResourceManager &resource_manager)
    : client_agent(std::move(client_agent)),
      client_communicator(std::move(client_communicator)),
      crt_post_proc_layer(std::make_unique<PostProcLayer>(resource_manager.shader_store.get("crt").get())),
      env(std::move(env)),
      io(io),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      resource_manager(resource_manager)
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
}

void MultiplayerScreen::update(double delta_time)
{
    bool finished = false;
    while (!finished)
    {
        std::string raw_message = client_communicator->get();
        if (raw_message.empty())
        {
            continue;
        }

        auto message_object = MsgPackCodec::decode<msgpack::object_handle>(raw_message);

        auto type = get_message_type(message_object.get());
        if (type == MessageType::GameStart)
        {
            auto message = message_object->as<GameStartMessage>();
            std::vector<nlohmann::json> body_specs;
            std::transform(message.body_specs.begin(), message.body_specs.end(),
                           std::back_inserter(body_specs),
                           [](const std::string &body_spec_string) {
                               return nlohmann::json::parse(body_spec_string);
                           });
            client_agent->set_bodies(body_specs);
            env->set_bodies(body_specs);
        }
        else if (type == MessageType::State)
        {
            auto message = message_object->as<StateMessage>();
            finished = message.done;

            auto action = client_agent->get_action(EnvState(message.agent_transforms,
                                                            message.entity_transforms,
                                                            message.tick));

            ActionMessage action_message(action, message.tick);
            auto encoded_action_message = MsgPackCodec::encode(action_message);
            client_communicator->send(encoded_action_message);

            env->add_new_state(EnvState(message.agent_transforms, message.entity_transforms, message.tick));
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

void MultiplayerScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.push_post_proc_layer(crt_post_proc_layer.get());
    renderer.begin();

    renderer.scissor(-10, -20, 10, 20, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f));
    auto render_data = env->get_render_data(lightweight);
    renderer.draw(render_data,
                  glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f),
                  env->get_env().get_elapsed_time(),
                  lightweight);

    auto crt_shader = resource_manager.shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
    crt_shader->set_uniform_1f("u_output_gamma", 1);
    crt_shader->set_uniform_1f("u_strength", 0.8);
    crt_shader->set_uniform_1f("u_distortion_factor", 0.1);

    renderer.end();
}
}