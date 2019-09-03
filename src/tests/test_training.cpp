#include <memory>
#include <string>

#include <doctest.h>
#include <imgui.h>

#include "misc/io.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/build_screen.h"
#include "screens/create_program_screen.h"
#include "screens/main_menu_screen.h"
#include "screens/multiplayer_screen.h"
#include "screens/train_screen.h"
#include "third_party/di.hpp"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"
#include "training/checkpointer.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "training/evaluators/elo_evaluator.h"
#include "training/saver.h"
#include "training/trainers/trainer.h"

using namespace SingularityTrainer;

namespace di = boost::di;

TEST_CASE("Training end-to-end test")
{
    const auto injector = di::make_injector(
        di::bind<int>.named(MaxSteps).to(100),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
        di::bind<ISaver>.to<Saver>(),
        di::bind<std::string>.named(CheckpointDirectory).to("/tmp/checkpoints"),
        di::bind<std::string>.named(AssetsPath).to("assets/"),
        di::bind<IScreenFactory>.named(
                                    BuildScreenFactoryType)
            .to<BuildScreenFactory>(),
        di::bind<IScreenFactory>.named(
                                    CreateProgramScreenFactoryType)
            .to<CreateProgramScreenFactory>(),
        di::bind<IScreenFactory>.named(
                                    MultiplayerScreenFactoryType)
            .to<MultiplayerScreenFactory>());

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &imgui_io = ImGui::GetIO();
    imgui_io.DisplaySize = ImVec2(1920, 1080);
    float delta_time = 1.f / 60.f;
    imgui_io.DeltaTime = delta_time;
    ImFontConfig font_config;
    imgui_io.Fonts->ClearFonts();
    imgui_io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 16, &font_config);
    imgui_io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 24, &font_config);
    imgui_io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 32, &font_config);
    unsigned char *tex_pixels = NULL;
    int tex_w, tex_h;
    imgui_io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);
    imgui_io.IniFilename = NULL;

    auto &io = injector.create<IO &>();
    io.set_resolution(1920, 1080);

    auto &screen_manager = injector.create<ScreenManager &>();
    auto &main_menu_screen_factory = injector.create<MainMenuScreenFactory &>();

    ImGui::NewFrame();
    screen_manager.show_screen(main_menu_screen_factory.make());
    screen_manager.update(delta_time);
    ImGui::Render();

    ImGui::NewFrame();
    std::dynamic_pointer_cast<MainMenuScreen>(screen_manager.current_screen())->train_agent();
    screen_manager.update(delta_time);
    ImGui::Render();

    ImGui::NewFrame();
    auto &program = std::dynamic_pointer_cast<CreateProgramScreen>(screen_manager.current_screen())
                        ->get_program();
    program.hyper_parameters.batch_size = 8;
    program.hyper_parameters.num_env = 2;
    program.hyper_parameters.num_minibatch = 2;
    program.minutes_per_checkpoint = 0;
    auto &test_body_factory = injector.create<TestBodyFactory &>();
    auto test_body = test_body_factory.make(injector.create<Random &>());
    program.body = test_body->to_json();
    std::dynamic_pointer_cast<CreateProgramScreen>(screen_manager.current_screen())
        ->run_training();
    screen_manager.update(delta_time);
    ImGui::Render();

    ImGui::NewFrame();
    std::dynamic_pointer_cast<TrainScreen>(screen_manager.current_screen())->set_fast(true);
    screen_manager.update(delta_time);
    ImGui::Render();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ImGui::DestroyContext();
}