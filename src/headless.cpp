#include <string>

#include "headless_app.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "third_party/di.hpp"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"
#include "training/checkpointer.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "training/evaluators/elo_evaluator.h"
#include "training/saver.h"
#include "training/trainer.h"

using namespace ai;

namespace di = boost::di;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<int>.named(MaxSteps).to(600),
        di::bind<IEnvironmentFactory>.to<KothEnvFactory>(),
        di::bind<ISaver>.to<Saver>(),
        di::bind<std::string>.named(CheckpointDirectory).to("checkpoints"));
    auto app = injector.create<HeadlessApp>();
    app.run(argc, argv);
}