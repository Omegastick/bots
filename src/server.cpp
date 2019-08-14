#include <string>

#include "server_app.h"
#include "third_party/di.hpp"
#include "training/environments/koth_env.h"
#include "third_party/zmq.hpp"

using namespace SingularityTrainer;

namespace di = boost::di;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<int>.named(MaxSteps).to(600),
        di::bind<double>.named(TickLength).to(0.1),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>());
    auto app = injector.create<ServerApp>();
    app.run(argc, argv);
}