#include <string>

#include "app.h"
#include "graphics/renderers/line_renderer.h"
#include "graphics/renderers/particle_renderer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/renderers/sprite_renderer.h"
#include "graphics/renderers/text_renderer.h"
#include "graphics/window.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/build_screen.h"
#include "screens/main_menu_screen.h"
#include "screens/training_wizard_screen.h"
#include "screens/watch_screen.h"
#include "third_party/di.hpp"

using namespace boost;
using namespace SingularityTrainer;

int main(int argc, char *argv[])
{
    const auto injector = di::make_injector(
        di::bind<int>.named(ResolutionX).to(1920),
        di::bind<int>.named(ResolutionY).to(1080),
        di::bind<std::string>.named(Title).to("Singularity Trainer"),
        di::bind<int>.named(MajorOpenGLVersion).to(4),
        di::bind<int>.named(MinorOpenGLVersion).to(3),
        di::bind<std::string>.named(AssetsPath).to("assets/"),
        di::bind<int>.named(MaxParticles).to(100000));
    auto app = injector.create<App>();
    app.run(argc, argv);
}