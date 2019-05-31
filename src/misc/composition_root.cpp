#include <memory>

#include "misc/composition_root.h"
#include "graphics/renderers/line_renderer.h"
#include "graphics/renderers/particle_renderer.h"
#include "graphics/renderers/renderer.h"
#include "graphics/renderers/sprite_renderer.h"
#include "graphics/renderers/text_renderer.h"
#include "graphics/window.h"
#include "misc/app.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/main_menu_screen.h"

namespace SingularityTrainer
{
CompositionRoot::CompositionRoot(int resolution_x, int resolution_y)
    : resolution_x(resolution_x), resolution_y(resolution_y) {}

std::unique_ptr<App> CompositionRoot::make_app()
{
    window = std::make_unique<Window>(resolution_x, resolution_y, "Singularity Trainer", 4, 3);
    io = std::make_unique<IO>();
    resource_manager = std::make_unique<ResourceManager>("assets/");
    screen_manager = std::make_unique<ScreenManager>();
    rng = std::make_unique<Random>(1);

    sprite_renderer = std::make_unique<SpriteRenderer>(*resource_manager);
    particle_renderer = std::make_unique<ParticleRenderer>(100000, *resource_manager);
    line_renderer = std::make_unique<LineRenderer>(*resource_manager);
    text_renderer = std::make_unique<TextRenderer>(*resource_manager);
    renderer = std::make_unique<Renderer>(resolution_x,
                                          resolution_y,
                                          *resource_manager,
                                          *sprite_renderer,
                                          *particle_renderer,
                                          *line_renderer,
                                          *text_renderer);

    main_menu_screen_factory = std::make_unique<MainMenuScreenFactory>(
        *resource_manager, *screen_manager, *io, *rng);

    return std::make_unique<App>(*io, *renderer, *main_menu_screen_factory, *screen_manager, *window);
}
}