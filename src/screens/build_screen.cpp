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
#include "io.h"
#include "random.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "ui/build_screen/part_selector_window.h"
#include "ui/build_screen/ship_builder.h"
#include "utilities.h"

namespace SingularityTrainer
{
BuildScreen::BuildScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, IO &io, Random &rng)
    : resource_manager(&resource_manager),
      screen_manager(&screen_manager),
      io(&io),
      part_selector_window(PartSelectorWindow(resource_manager)),
      available_parts({"base_module", "gun_module", "thruster_module", "laser_sensor_module"}),
      selected_part(),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      b2_world(b2Vec2(0, 0)),
      ship_builder(b2_world, rng, io),
      module_to_place(nullptr),
      test_sprite("laser_sensor_module")
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    for (const auto &part : available_parts)
    {
        resource_manager.load_texture(part, "images/" + part + ".png");
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
    auto selected_part_ = part_selector_window.update(available_parts);
    if (selected_part_ != "")
    {
        selected_part = selected_part_;
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

    if (io->get_key_pressed_this_frame(GLFW_KEY_Q))
    {
        module_to_place->get_transform().q = b2Mul(module_to_place->get_transform().q, b2Rot(glm::radians(90.f)));
    }
    else if (io->get_key_pressed_this_frame(GLFW_KEY_E))
    {
        module_to_place->get_transform().q = b2Mul(module_to_place->get_transform().q, b2Rot(glm::radians(-90.f)));
    }
}

void BuildScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

    RenderData render_data;

    if (selected_part != "")
    {
        auto cursor_world_position = screen_to_world_space(io->get_cursor_position(),
                                                           io->get_resolution(),
                                                           ship_builder.get_projection());
        module_to_place->get_transform().p = {cursor_world_position.x, cursor_world_position.y};
        auto nearest_link_result = ship_builder.get_nearest_module_link_to_module(*module_to_place);
        if (nearest_link_result.nearest_link != nullptr && nearest_link_result.distance < 1)
        {
            nearest_link_result.origin_link->snap_to_other(*nearest_link_result.nearest_link);
        }

        render_data.append(module_to_place->get_render_data());
    }

    render_data.append(ship_builder.get_render_data());

    for (const auto &module : ship_builder.get_agent()->get_modules())
    {
        for (const auto &link : module->get_module_links())
        {
            auto transform = link.get_global_transform();
            test_sprite.set_position({transform.p.x, transform.p.y});
            test_sprite.set_rotation(transform.q.GetAngle());
            renderer.draw(test_sprite, ship_builder.get_projection());
        }
    }

    renderer.draw(render_data, ship_builder.get_projection(), 0., lightweight);

    renderer.end();
}
}