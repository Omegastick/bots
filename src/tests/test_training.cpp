#include <memory>
#include <filesystem>
#include <string>

#include <doctest.h>
#include <imgui.h>

#include "misc/io.h"
#include "misc/credentials_manager.h"
#include "misc/ihttp_client.h"
#include "misc/matchmaker.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/build_screen.h"
#include "screens/create_program_screen.h"
#include "screens/main_menu_screen.h"
#include "screens/multiplayer_screen.h"
#include "screens/train_screen.h"
#include "tests/e2e_framework.h"
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
namespace fs = std::filesystem;

TEST_CASE("E2E - Training run")
{
    float delta_time = 1.f / 60.f;
    setup_imgui(delta_time);

    const auto injector = di::make_injector(
        di::bind<int>.named(MaxSteps).to(100),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
        di::bind<ISaver>.to<Saver>(),
        di::bind<std::string>.named(CheckpointDirectory).to("/tmp/checkpoints"),
        di::bind<std::string>.named(AssetsPath).to("assets/"),
        di::bind<IHttpClient>.to<MockHttpClient>(),
        di::bind<IScreenFactory>.named(
                                    BuildScreenFactoryType)
            .to<BuildScreenFactory>(),
        di::bind<IScreenFactory>.named(
                                    CreateProgramScreenFactoryType)
            .to<CreateProgramScreenFactory>(),
        di::bind<IScreenFactory>.named(
                                    MultiplayerScreenFactoryType)
            .to<MultiplayerScreenFactory>());

    auto &io = injector.create<IO &>();
    io.set_resolution(1920, 1080);

    fs::remove_all("/tmp/checkpoints");

    auto &screen_manager = injector.create<ScreenManager &>();
    auto &main_menu_screen_factory = injector.create<MainMenuScreenFactory &>();

    {
        Frame frame(delta_time, screen_manager);
        screen_manager.show_screen(main_menu_screen_factory.make());
    }

    {
        Frame frame(delta_time, screen_manager);
        std::dynamic_pointer_cast<MainMenuScreen>(screen_manager.current_screen())->train_agent();
    }

    {
        Frame frame(delta_time, screen_manager);
        auto create_program_screen = std::dynamic_pointer_cast<CreateProgramScreen>(
            screen_manager.current_screen());
        auto &program = create_program_screen->get_program();
        program.hyper_parameters.batch_size = 2;
        program.hyper_parameters.num_env = 2;
        program.hyper_parameters.num_minibatch = 1;
        program.minutes_per_checkpoint = 0;
        auto &test_body_factory = injector.create<TestBodyFactory &>();
        auto test_body = test_body_factory.make(injector.create<Random &>());
        program.body = test_body->to_json();
        std::dynamic_pointer_cast<CreateProgramScreen>(screen_manager.current_screen())
            ->run_training();
    }

    {
        Frame frame(delta_time, screen_manager);
        std::dynamic_pointer_cast<TrainScreen>(screen_manager.current_screen())->set_fast(true);
    }

    screen_manager.exit();

    DOCTEST_CHECK(std::distance(fs::directory_iterator("/tmp/checkpoints"),
                                fs::directory_iterator{}) == 2);

    fs::remove_all("/tmp/checkpoints");
    ImGui::DestroyContext();
}