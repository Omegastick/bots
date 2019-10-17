#include <memory>
#include <stdexcept>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "create_program_screen.h"
#include "graphics/backend/shader.h"
#include "graphics/renderers/renderer.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/train_screen.h"
#include "training/bodies/body.h"
#include "training/checkpointer.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "training/training_program.h"
#include "ui/back_button.h"
#include "ui/create_program_screen/algorithm_window.h"
#include "ui/create_program_screen/body_selector_window.h"
#include "ui/create_program_screen/brain_window.h"
#include "ui/create_program_screen/reward_windows.h"
#include "ui/create_program_screen/save_load_window.h"
#include "ui/create_program_screen/tabs.h"

namespace SingularityTrainer
{

CreateProgramScreen::CreateProgramScreen(std::unique_ptr<AlgorithmWindow> algorithm_window,
                                         std::unique_ptr<BodySelectorWindow> body_selector_window,
                                         std::unique_ptr<BrainWindow> brain_window,
                                         std::unique_ptr<RewardWindows> reward_windows,
                                         std::unique_ptr<IEnvironment> environment,
                                         std::unique_ptr<TrainingProgram> program,
                                         std::unique_ptr<SaveLoadWindow> save_load_window,
                                         std::unique_ptr<Tabs> tabs,
                                         IO &io,
                                         ResourceManager &resource_manager,
                                         ScreenManager &screen_manager,
                                         TrainScreenFactory &train_screen_factory)
    : algorithm_window(std::move(algorithm_window)),
      body_selector_window(std::move(body_selector_window)),
      brain_window(std::move(brain_window)),
      environment(std::move(environment)),
      io(io),
      program(std::move(program)),
      resource_manager(resource_manager),
      reward_windows(std::move(reward_windows)),
      save_load_window(std::move(save_load_window)),
      screen_manager(screen_manager),
      state(CreateProgramScreenState::Body),
      tabs(std::move(tabs)),
      train_screen_factory(train_screen_factory)
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("square_hull", "images/square_hull.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);

    crt_post_proc_layer = std::make_unique<PostProcLayer>(
        *resource_manager.shader_store.get("crt"),
        io.get_resolution().x,
        io.get_resolution().y);
}

void CreateProgramScreen::algorithm()
{
    algorithm_window->update(program->hyper_parameters);
}

void CreateProgramScreen::body()
{
    auto json = body_selector_window->update();
    if (!json.empty())
    {
        auto bodies = environment->get_bodies();
        try
        {
            bodies[0]->load_json(json);
            bodies[1]->load_json(json);
        }
        catch (std::runtime_error &ex)
        {
            spdlog::error("Can't load JSON");
        }

        environment->reset();
        program->body = json;
    }
}

void CreateProgramScreen::brain()
{
    brain_window->update(*program);
}

void CreateProgramScreen::rewards()
{
    reward_windows->update(*environment, projection, program->reward_config);
}

void CreateProgramScreen::save_load()
{
    if (save_load_window->update(*program))
    {
        auto bodies = environment->get_bodies();
        bodies[0]->load_json(program->body);
        bodies[1]->load_json(program->body);
        environment->reset();
    }
}

void CreateProgramScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.set_view(projection);
    
    renderer.push_post_proc_layer(*crt_post_proc_layer);

    environment->draw(renderer, lightweight);

    auto crt_shader = resource_manager.shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
    crt_shader->set_uniform_1f("u_output_gamma", 1);
    crt_shader->set_uniform_1f("u_strength", 0.3);
    crt_shader->set_uniform_1f("u_distortion_factor", 0.05);
}

void CreateProgramScreen::update(double /*delta_time*/)
{
    const double view_height = 50;
    auto view_top = view_height * 0.5;
    glm::vec2 resolution = io.get_resolution();
    auto view_right = view_top * (resolution.x / resolution.y);
    projection = glm::ortho(-view_right, view_right, -view_top, view_top);

    state = tabs->update();

    switch (state)
    {
    case CreateProgramScreenState::Algorithm:
        algorithm();
        break;
    case CreateProgramScreenState::Body:
        body();
        break;
    case CreateProgramScreenState::Brain:
        brain();
        break;
    case CreateProgramScreenState::Rewards:
        rewards();
        break;
    case CreateProgramScreenState::SaveLoad:
        save_load();
        break;
    case CreateProgramScreenState::Opponents:
    case CreateProgramScreenState::Schedule:
        break;
    }

    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0, 0, 0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {12, 6});
    auto imgui_io = ImGui::GetIO();
    ImGui::PushFont(imgui_io.Fonts->Fonts[2]);
    ImGui::SetNextWindowPos({resolution.x, resolution.y}, ImGuiCond_Always, {1, 1});
    ImGui::Begin("##run_training", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
    if (ImGui::Button("Run"))
    {
        run_training();
    }
    ImGui::End();
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    back_button(screen_manager, resolution);
}

void CreateProgramScreen::run_training()
{
    screen_manager.close_screen();
    screen_manager.show_screen(train_screen_factory.make(*program));
}

std::shared_ptr<IScreen> CreateProgramScreenFactory::make()
{
    auto world = std::make_unique<b2World>(b2Vec2_zero);
    auto rng = std::make_unique<Random>(0);
    std::vector<std::unique_ptr<Body>> bodies;
    bodies.push_back(body_factory.make(*world, *rng));
    bodies.push_back(body_factory.make(*world, *rng));
    auto environment = env_factory.make(std::move(rng),
                                        std::move(world),
                                        std::move(bodies),
                                        RewardConfig());
    return std::make_shared<CreateProgramScreen>(std::make_unique<AlgorithmWindow>(io),
                                                 std::make_unique<BodySelectorWindow>(io),
                                                 std::make_unique<BrainWindow>(checkpointer, io),
                                                 std::make_unique<RewardWindows>(io),
                                                 std::move(environment),
                                                 std::make_unique<TrainingProgram>(),
                                                 std::make_unique<SaveLoadWindow>(io),
                                                 std::make_unique<Tabs>(io),
                                                 io,
                                                 resource_manager,
                                                 screen_manager,
                                                 train_screen_factory);
}
}