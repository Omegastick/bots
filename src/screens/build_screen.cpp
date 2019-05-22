#include <memory>

#include <Box2D/Box2D.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "screens/build_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "graphics/sprite.h"
#include "io.h"
#include "random.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "ui/build_screen/part_selector_window.h"
#include "ui/build_screen/ship_builder.h"

namespace SingularityTrainer
{
BuildScreen::BuildScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, IO &io, Random &rng)
    : resource_manager(&resource_manager),
      screen_manager(&screen_manager),
      io(&io),
      part_selector_window(PartSelectorWindow(resource_manager)),
      available_parts({"base_module", "gun_module", "thruster_module", "laser_sensor_module"}),
      selected_part(),
      ghost(Sprite("")),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      b2_world(b2World(b2Vec2(0, 0))),
      ship_builder(ShipBuilder(b2_world, rng, io)),
      module_to_place(nullptr)
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    for (const auto &part : available_parts)
    {
        resource_manager.load_texture(part, "images/" + part + ".png");
    }
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);
}

void BuildScreen::update(double /*delta_time*/)
{
    auto selected_part_ = part_selector_window.update(available_parts);
    if (selected_part_ != "")
    {
        selected_part = selected_part_;
        ghost.set_texture(selected_part);
        ghost.set_scale(glm::vec2(100, 100));
        ghost.set_origin(ghost.get_center());
        ghost.set_color(cl_white);
        module_to_place = std::make_shared<GunModule>();
    }

    if (io->get_left_click())
    {
        if (ship_builder.click(module_to_place) != nullptr)
        {
            module_to_place == nullptr;
            selected_part = "";
        }
    }
}

void BuildScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

    if (selected_part != "")
    {
        auto cursor_pos = io->get_cursor_position();
        cursor_pos *= glm::vec2(1920, 1080) / static_cast<glm::vec2>(io->get_resolution());
        ghost.set_position({cursor_pos.x, cursor_pos.y});
        renderer.draw(ghost, projection);
    }

    auto ship_builder_render_data = ship_builder.get_render_data();
    renderer.draw(ship_builder_render_data, ship_builder.get_projection(), 0., lightweight);

    renderer.end();
}
}