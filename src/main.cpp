#include "misc/composition_root.h"
#include "misc/app.h"

using namespace SingularityTrainer;

int main(int argc, char *argv[])
{
    CompositionRoot composition_root(1920, 1080);
    auto app = composition_root.make_app();
    app->run(argc, argv);
}