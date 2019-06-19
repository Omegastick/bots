#include <string>

#include "headless_app.h"
#include "misc/random.h"
#include "third_party/di.hpp"
#include "training/agents/agent.h"
#include "training/agents/test_agent.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "training/trainers/itrainer.h"
#include "training/trainers/trainer.h"

using namespace SingularityTrainer;

namespace di = boost::di;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<ITrainer>.to<Trainer>(),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
        di::bind<AgentFactory>.to<TestAgentFactory>());
    auto app = injector.create<HeadlessApp>();
    app.run(argc, argv);
}