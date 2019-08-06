#include <string>

#include <zmq.hpp>

#include "server_app.h"
#include "third_party/di.hpp"
#include "training/environments/koth_env.h"

using namespace SingularityTrainer;

namespace di = boost::di;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<int>.named(MaxSteps).to(600),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>());
    auto app = injector.create<ServerApp>();
    app.run(argc, argv);
}