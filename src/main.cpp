#include <chrono>
#include <string>
#include <sstream>

#include "app.h"
#include "audio/audio_engine.h"
#include "graphics/renderers/line_renderer.h"
#include "graphics/renderers/particle_renderer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/renderers/batched_sprite_renderer.h"
#include "graphics/renderers/text_renderer.h"
#include "graphics/renderers/vector_renderer.h"
#include "graphics/window.h"
#include "misc/animator.h"
#include "misc/credentials_manager.h"
#include "misc/http_client.h"
#include "misc/io.h"
#include "misc/matchmaker.h"
#include "misc/module_factory.h"
#include "misc/module_texture_store.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/build_screen.h"
#include "screens/main_menu_screen.h"
#include "screens/create_program_screen.h"
#include "screens/multiplayer_screen.h"
#include "screens/train_screen.h"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"
#include "training/checkpointer.h"
#include "training/entities/bullet.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "training/environments/playback_env.h"
#include "training/evaluators/elo_evaluator.h"
#include "training/saver.h"
#include "third_party/di.hpp"
#include "ui/background.h"

using namespace ai;

namespace di = boost::di;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<int>.named(RandomSeed).to(static_cast<int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())),
        di::bind<int>.named(ResolutionX).to(1920),
        di::bind<int>.named(ResolutionY).to(1080),
        di::bind<std::string>.named(Title).to("AI: Artificial Insentience"),
        di::bind<int>.named(MajorOpenGLVersion).to(4),
        di::bind<int>.named(MinorOpenGLVersion).to(3),
        di::bind<std::string>.named(AssetsPath).to("assets/"),
        di::bind<int>.named(MaxParticles).to(100000),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
        di::bind<int>.named(MaxSteps).to(600),
        di::bind<double>.named(TickLength).to(0.1),
        di::bind<ISaver>.to<Saver>(),
        di::bind<std::string>.named(CheckpointDirectory).to("checkpoints"),
        di::bind<IHttpClient>.to<HttpClient>(),
        di::bind<BodyFactory>.to<BodyFactory>(),
        di::bind<IScreenFactory>.named(BuildScreenFactoryType).to<BuildScreenFactory>(),
        di::bind<IScreenFactory>.named(CreateProgramScreenFactoryType).to<CreateProgramScreenFactory>(),
        di::bind<IScreenFactory>.named(MultiplayerScreenFactoryType).to<MultiplayerScreenFactory>(),
        di::bind<IAudioEngine>.to<AudioEngine>(),
        di::bind<IModuleFactory>.to<ModuleFactory>(),
        di::bind<IBulletFactory>.to<BulletFactory>());
    auto app = injector.create<App>();
    return app.run(argc, argv);
}