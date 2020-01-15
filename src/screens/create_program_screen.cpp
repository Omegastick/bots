#include <memory>
#include <stdexcept>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "create_program_screen.h"
#include "audio/audio_engine.h"
#include "graphics/backend/shader.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "graphics/post_processing/bloom_layer.h"
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
#include "ui/create_program_screen/create_program_screen_ui.h"

namespace ai
{

CreateProgramScreen::CreateProgramScreen(std::unique_ptr<CreateProgramScreenUI> ui,
                                         std::unique_ptr<IEnvironment> environment,
                                         std::unique_ptr<TrainingProgram> program,
                                         AudioEngine &audio_engine,
                                         IO &io,
                                         ResourceManager &resource_manager,
                                         ScreenManager &screen_manager,
                                         TrainScreenFactory &train_screen_factory)
    : audio_engine(audio_engine),
      environment(std::move(environment)),
      io(io),
      program(std::move(program)),
      resource_manager(resource_manager),
      screen_manager(screen_manager),
      state(CreateProgramScreenState::Body),
      train_screen_factory(train_screen_factory),
      ui(std::move(ui))
{
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);
    resource_manager.load_font("roboto-32", "fonts/Roboto-Regular.ttf", 32);
}

void CreateProgramScreen::algorithm()
{
    ui->algorithm_window.update(program->hyper_parameters);
}

void CreateProgramScreen::body()
{
    auto json = ui->body_selector_window.update();
    if (!json.empty())
    {
        auto bodies = environment->get_bodies();
        try
        {
            bodies[0]->load_json(json);
            bodies[1]->load_json(json);
            bodies[1]->set_color({cl_red, set_alpha(cl_red, 0.2f)});
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
    ui->brain_window.update(*program);
}

void CreateProgramScreen::rewards()
{
    ui->reward_windows.update(*environment, projection, program->reward_config);
}

void CreateProgramScreen::save_load()
{
    if (ui->save_load_window.update(*program))
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

    environment->draw(renderer, lightweight);
}

void CreateProgramScreen::update(double /*delta_time*/)
{
    const double view_height = 50;
    auto view_top = view_height * 0.5;
    glm::vec2 resolution = io.get_resolution();
    auto view_right = view_top * (resolution.x / resolution.y);
    projection = glm::ortho(-view_right, view_right, -view_top, view_top);

    state = ui->tabs.update();

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
        audio_engine.play("hit");
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
    return std::make_shared<CreateProgramScreen>(ui_factory.make(),
                                                 std::move(environment),
                                                 std::make_unique<TrainingProgram>(),
                                                 audio_engine,
                                                 io,
                                                 resource_manager,
                                                 screen_manager,
                                                 train_screen_factory);
}
}