#include <memory>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <Box2D/Box2D.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "screens/build_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "graphics/sprite.h"
#include "misc/io.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "misc/utilities.h"
#include "ui/build_screen/part_selector_window.h"
#include "ui/build_screen/ship_builder.h"

namespace SingularityTrainer
{
BuildScreen::BuildScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, IO &io, Random &rng)
    : resource_manager(&resource_manager),
      screen_manager(&screen_manager),
      io(&io),
      part_detail_window(io),
      part_selector_window(io, resource_manager),
      available_parts({"base", "gun", "thruster", "laser_sensor"}),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      b2_world(b2Vec2(0, 0)),
      save_ship_window(io),
      ship_builder(b2_world, rng, io),
      module_to_place(nullptr),
      test_sprite("laser_sensor_module"),
      current_rotation(0)
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    for (const auto &part : available_parts)
    {
        resource_manager.load_texture(part + "_module", "images/" + part + "_module.png");
    }
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);

    test_sprite.set_scale({0.2, 0.2});
    test_sprite.set_origin(test_sprite.get_center());
    test_sprite.set_color(cl_white);
}

void BuildScreen::update(double /*delta_time*/)
{
    auto selected_part = part_selector_window.update(available_parts);
    if (selected_part != "")
    {
        module_to_place = ModuleFactory::create_module(selected_part);
    }

    if (io->get_left_click())
    {
        if (module_to_place == nullptr)
        {
            selected_module = ship_builder.get_module_at_screen_position(io->get_cursor_position());
        }
        else
        {
            selected_module = ship_builder.place_module(module_to_place);
        }

        if (selected_module != nullptr)
        {
            module_to_place = nullptr;
            part_detail_window.select_part(selected_module.get());
        }
    }

    if (io->get_key_pressed_this_frame(GLFW_KEY_Q))
    {
        current_rotation += 1;
    }
    else if (io->get_key_pressed_this_frame(GLFW_KEY_E))
    {
        current_rotation -= 1;
    }

    part_detail_window.update();
    save_ship_window.update(*ship_builder.get_agent());
}

void BuildScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

    RenderData render_data;

    if (module_to_place != nullptr)
    {
        auto cursor_world_position = screen_to_world_space(io->get_cursor_position(),
                                                           io->get_resolution(),
                                                           ship_builder.get_projection());
        module_to_place->get_transform().p = {cursor_world_position.x, cursor_world_position.y};
        module_to_place->get_transform().q = b2Rot(glm::radians(current_rotation * 90.f));

        auto nearest_link_result = ship_builder.get_nearest_module_link_to_module(*module_to_place);
        if (nearest_link_result.nearest_link != nullptr && nearest_link_result.distance < 1)
        {
            nearest_link_result.origin_link->snap_to_other(*nearest_link_result.nearest_link);
        }

        render_data.append(module_to_place->get_render_data());
    }

    render_data.append(ship_builder.get_render_data());

    renderer.draw(render_data, ship_builder.get_projection(), 0., lightweight);

    renderer.end();
}
}