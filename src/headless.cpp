#include <string>

#include "headless_app.h"
#include "misc/random.h"
#include "third_party/di.hpp"
#include "training/agents/agent.h"
#include "training/agents/test_agent.h"
#include "training/checkpointer.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "training/saver.h"
#include "training/trainers/itrainer.h"
#include "training/trainers/trainer.h"

using namespace SingularityTrainer;

namespace di = boost::di;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
        di::bind<ISaver>.to<Saver>(),
        di::bind<std::string>.named(CheckpointDirectory).to("checkpoints"));
    auto app = injector.create<HeadlessApp>();
    app.run(argc, argv);
}