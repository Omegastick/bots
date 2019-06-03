#include <string>

#include "headless_app.h"
#include "misc/random.h"
#include "third_party/di.hpp"
#include "training/trainers/itrainer.h"
#include "training/trainers/quick_trainer.h"

using namespace SingularityTrainer;

namespace di = boost::di;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<ITrainer>.to<QuickTrainer>(),
        di::bind<int>.named(EnvCount).to(atoi(argv[1])));
    auto app = injector.create<HeadlessApp>();
    app.run(argc, argv);
}