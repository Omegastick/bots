#include <iostream>
#include <string>

#include "server_app.h"
#include "audio/audio_engine.h"
#include "misc/module_factory.h"
#include "misc/resource_manager.h"
#include "third_party/di.hpp"
#include "training/entities/bullet.h"
#include "training/environments/koth_env.h"
#include "third_party/zmq.hpp"

using namespace ai;

namespace di = boost::di;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<int>.named(MaxSteps).to(600),
        di::bind<double>.named(TickLength).to(0.1),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
        di::bind<IAudioEngine>.to<AudioEngine>(),
        di::bind<IModuleFactory>.to<ModuleFactory>(),
        di::bind<IBulletFactory>.to<BulletFactory>());
    auto app = injector.create<ServerApp>();
    app.run(argc, argv);
}