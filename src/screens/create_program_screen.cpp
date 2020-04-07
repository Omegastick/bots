#include <memory>
#include <stdexcept>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "create_program_screen.h"
#include "audio/audio_engine.h"
#include "environment/ecs_env.h"
#include "environment/iecs_env.h"
#include "graphics/backend/shader.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "graphics/post_processing/bloom_layer.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/train_screen.h"
#include "training/checkpointer.h"
#include "training/training_program.h"
#include "ui/back_button.h"
#include "ui/create_program_screen/create_program_screen_ui.h"

namespace ai
{

CreateProgramScreen::CreateProgramScreen(std::unique_ptr<CreateProgramScreenUI> ui,
                                         std::unique_ptr<IEcsEnv> environment,
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
    const auto json = ui->body_selector_window.update();
    if (!json.empty())
    {
        try
        {
            environment->set_body(0, json);

            auto opponent_json = json;
            opponent_json["color_scheme"]["primary"] = {cl_red.r, cl_red.g, cl_red.b, cl_red.a};
            const auto transparent_red = set_alpha(cl_red, 0.2f);
            opponent_json["color_scheme"]["secondary"] = {transparent_red.r,
                                                          transparent_red.g,
                                                          transparent_red.b,
                                                          transparent_red.a};
            environment->set_body(1, opponent_json);
            program->body = json;
        }
        catch (const std::runtime_error &ex)
        {
            spdlog::error("Could not load Json: {}", ex.what());
        }

        environment->reset();
    }
}

void CreateProgramScreen::brain()
{
    ui->brain_window.update(*program);
}

void CreateProgramScreen::rewards()
{
    ui->reward_windows.update(projection, program->reward_config);
}

void CreateProgramScreen::save_load()
{
    if (ui->save_load_window.update(*program))
    {
        environment->set_body(0, program->body);

        auto opponent_json = program->body;
        opponent_json["color_scheme"]["primary"] = {cl_red.r, cl_red.g, cl_red.b, cl_red.a};
        const auto transparent_red = set_alpha(cl_red, 0.2f);
        opponent_json["color_scheme"]["secondary"] = {transparent_red.r,
                                                      transparent_red.g,
                                                      transparent_red.b,
                                                      transparent_red.a};
        environment->set_body(1, opponent_json);
        environment->reset();
    }
}

void CreateProgramScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.set_view(projection);

    environment->draw(renderer, audio_engine, lightweight);
}

void CreateProgramScreen::update(double delta_time)
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
        audio_engine.play("chord");
        run_training();
    }
    ImGui::End();
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    back_button(screen_manager, resolution);

    environment->forward(delta_time);
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
    return std::make_shared<CreateProgramScreen>(ui_factory.make(),
                                                 std::make_unique<EcsEnv>(),
                                                 std::make_unique<TrainingProgram>(),
                                                 audio_engine,
                                                 io,
                                                 resource_manager,
                                                 screen_manager,
                                                 train_screen_factory);
}
}