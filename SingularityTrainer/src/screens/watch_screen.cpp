#include <memory>
#include <filesystem>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "screens/watch_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/backend/shader.h"
#include "graphics/post_proc_layer.h"
#include "graphics/colors.h"
#include "training/environments/koth_env.h"
#include "training/trainers/quick_trainer.h"
#include "communicator.h"
#include "iscreen.h"
#include "resource_manager.h"

namespace fs = std::filesystem;

namespace SingularityTrainer
{
WatchScreen::WatchScreen(ResourceManager &resource_manager, Communicator &communicator, Random &rng)
    : projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)), resource_manager(&resource_manager), state(States::BROWSING)
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
}

void WatchScreen::update(const float delta_time)
{
    if (state == States::BROWSING)
    {
        ImGui::SetNextWindowPosCenter();
        ImGui::Begin("Pick a checkpoint", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

        std::vector<std::string> files;
        for (const auto &file : fs::directory_iterator(fs::current_path().parent_path()))
        {
            if (file.path().extension().string() == ".pth")
            {
                auto filename = file.path().filename().string();
                files.push_back(filename.substr(0, filename.size() - 4));
            }
        }
        std::vector<char *> c_strings{};

        for (auto &string : files)
        {
            c_strings.push_back(&string.front());
        }

        static int selected_item = -1;
        ImGui::ListBox("", &selected_item, &c_strings.front(), files.size());

        ImGui::Button("Select");
        ImGui::End();
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
}