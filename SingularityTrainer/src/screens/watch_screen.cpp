#include <memory>
#include <filesystem>
#include <future>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "screens/watch_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/post_proc_layer.h"
#include "training/environments/koth_env.h"
#include "communicator.h"
#include "resource_manager.h"
#include "requests.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
WatchScreen::WatchScreen(ResourceManager &resource_manager, Communicator &communicator, Random &rng)
    : projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      resource_manager(&resource_manager),
      communicator(&communicator),
      state(States::BROWSING),
      selected_file(-1),
      frame_counter(0)
{
    environment = std::make_unique<KothEnv>(460, 40, 1, 100, 0);

    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");

    crt_post_proc_layer = std::make_unique<PostProcLayer>(resource_manager.shader_store.get("crt").get());
}

WatchScreen::~WatchScreen()
{
    if (state == States::WATCHING)
    {
        std::shared_ptr<EndSessionParam> end_session_param = std::make_shared<EndSessionParam>();
        end_session_param->session_id = 0;
        Request<EndSessionParam> end_session_request("end_session", end_session_param, 4);
        communicator->send_request<EndSessionParam>(end_session_request);
        communicator->get_response<EndSessionResult>();
    }
}

void WatchScreen::update(const float delta_time)
{
    if (state == States::BROWSING)
    {
        // Load agent window
        ImGui::SetNextWindowPosCenter();
        ImGui::Begin("Pick a checkpoint", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

        // Enumerate all model files
        std::vector<std::string> files;
        for (const auto &file : fs::directory_iterator(fs::current_path().parent_path()))
        {
            if (file.path().extension().string() == ".pth")
            {
                auto filename = file.path().filename().string();
                files.push_back(filename.substr(0, filename.size() - 4));
            }
        }
        // Display list of model files
        std::vector<char *> c_strings;
        for (auto &string : files)
        {
            c_strings.push_back(&string.front());
        }
        ImGui::ListBox("", &selected_file, &c_strings.front(), files.size());

        if (ImGui::Button("Select"))
        {
            auto begin_session_param = std::make_shared<BeginSessionParam>();
            Model model{12, 4, true, true};
            begin_session_param->model = model;
            begin_session_param->contexts = 2;
            begin_session_param->session_id = 0;
            begin_session_param->training = false;
            begin_session_param->model_path = fs::current_path().parent_path().append(files[selected_file] + ".pth");
            Request<BeginSessionParam> request("begin_session", begin_session_param, 0);
            communicator->send_request<BeginSessionParam>(request);
            communicator->get_response<BeginSessionResult>();

            environment->start_thread();
            observations = environment->reset().get()->observation;

            state = States::WATCHING;
        }
        ImGui::End();
    }
    else if (state == States::WATCHING)
    {
        if (++frame_counter >= 6)
        {
            frame_counter = 0;
            action_update();
        }
        else
        {
            environment->forward(1. / 60.);
        }
    }
}

void WatchScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.push_post_proc_layer(crt_post_proc_layer.get());
    renderer.begin();

    if (state == States::WATCHING)
    {
        renderer.scissor(-10, -20, 10, 20, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f));
        auto render_data = environment->get_render_data();
        renderer.draw(render_data, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f), environment->get_elapsed_time());

        auto crt_shader = resource_manager->shader_store.get("crt");
        crt_shader->set_uniform_2f("u_resolution", glm::vec2(renderer.get_width(), renderer.get_height()));
        crt_shader->set_uniform_1f("u_output_gamma", 1);
        crt_shader->set_uniform_1f("u_strength", 0.8);
        crt_shader->set_uniform_1f("u_distortion_factor", 0.03);
    }

    renderer.end();
}

void WatchScreen::action_update()
{
    // Get actions
    auto get_actions_param = std::make_shared<GetActionsParam>();
    get_actions_param->inputs = {observations};
    get_actions_param->session_id = 0;
    Request<GetActionsParam> get_actions_request("get_actions", get_actions_param, 2);
    communicator->send_request(get_actions_request);
    auto actions = communicator->get_response<GetActionsResult>()->result.actions;

    // Step environment
    auto step_info = environment->step(actions, 1. / 60.).get();
    observations = step_info->observation;
}
}