#include <algorithm>
#include <memory>
#include <filesystem>
#include <future>

#include <cpprl/cpprl.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/fmt/fmt.h>

#include "screens/watch_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/post_proc_layer.h"
#include "training/environments/target_env.h"
#include "resource_manager.h"
#include "requests.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
WatchScreen::WatchScreen(ResourceManager &resource_manager, Random & /*rng*/)
    : policy(nullptr),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      resource_manager(&resource_manager),
      state(States::BROWSING),
      selected_file(-1),
      frame_counter(0),
      scores({{0}})
{
    environment = std::make_unique<TargetEnv>(460, 40, 1, 100, 0);

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

WatchScreen::~WatchScreen() {}

void WatchScreen::update(const double /*delta_time*/)
{
    if (state == States::BROWSING)
    {
        show_checkpoint_selector();
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

        show_agent_scores();
    }
}

void WatchScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.push_post_proc_layer(crt_post_proc_layer.get());
    renderer.begin();

    if (state == States::WATCHING)
    {
        renderer.scissor(-10, -20, 10, 20, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f));
        auto render_data = environment->get_render_data();
        renderer.draw(render_data, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f), environment->get_elapsed_time());

        auto crt_shader = resource_manager->shader_store.get("crt");
        crt_shader->set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
        crt_shader->set_uniform_1f("u_output_gamma", 1);
        crt_shader->set_uniform_1f("u_strength", 0.8);
        crt_shader->set_uniform_1f("u_distortion_factor", 0.1);
    }

    renderer.end();
}

void WatchScreen::action_update()
{
    // Get actions from policy
    std::vector<torch::Tensor> act_result;
    {
        torch::NoGradGuard no_grad;
        act_result = policy->act(observations,
                                 hidden_states,
                                 masks);
    }
    hidden_states = act_result[3];

    // Step environment
    auto step_info = environment->step(act_result[1], 1. / 60.).get();
    observations = step_info.observation.view({1, -1});
    for (unsigned int i = 0; i < scores.size(); ++i)
    {
        scores[i].push_back(scores[i].back() + step_info.reward[i].item().toFloat());
    }
    if (step_info.done[0].item().toBool())
    {
        for (auto &agent_scores : scores)
        {
            agent_scores = {0};
        }
    }
}

void WatchScreen::show_checkpoint_selector()
{
    // Load agent window
    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    ImGui::Begin("Pick a checkpoint", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

    // Enumerate all model files
    std::vector<std::string> files;
    for (const auto &file : fs::directory_iterator(fs::current_path()))
    {
        if (file.path().extension().string() == ".pth")
        {
            auto filename = file.path().filename().string();
            files.push_back(filename.substr(0, filename.size() - 4));
        }
    }
    std::sort(files.begin(), files.end());
    // Display list of model files
    std::vector<char *> c_strings;
    for (auto &string : files)
    {
        c_strings.push_back(&string.front());
    }
    ImGui::ListBox("", &selected_file, &c_strings.front(), files.size());

    if (ImGui::Button("Select"))
    {
        environment->start_thread();
        observations = environment->reset().get().observation.view({1, -1});
        masks = torch::zeros({1, 1});
        hidden_states = torch::zeros({1, 24});

        nn_base = std::make_shared<cpprl::MlpBase>(23, false, 24);
        policy = cpprl::Policy(cpprl::ActionSpace{"MultiBinary", {4}}, nn_base);
        torch::load(policy, files[selected_file] + ".pth");

        state = States::WATCHING;
    }
    ImGui::End();
}

void WatchScreen::show_agent_scores()
{
    for (unsigned int i = 0; i < scores.size(); ++i)
    {
        std::string title = fmt::format("Agent {}", i + 1);
        ImGui::Begin(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
        ImGui::Text("Score: %.0f", scores[0].back());
        ImGui::PlotLines("", &scores[0].front(), scores[0].size());
        ImGui::End();
    }
}
}