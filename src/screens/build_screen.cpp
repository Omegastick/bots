#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "graphics/renderers/renderer.h"
#include "graphics/sprite.h"
#include "screens/build_screen.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "ui/build_screen/part_selector_window.h"

namespace SingularityTrainer
{
BuildScreen::BuildScreen(ResourceManager &resource_manager, ScreenManager &screen_manager)
    : resource_manager(&resource_manager),
      screen_manager(&screen_manager),
      part_selector_window(std::make_unique<PartSelectorWindow>(resource_manager)),
      available_parts({"base_module", "gun_module", "thruster_module", "laser_sensor_module"}),
      selected_part(),
      ghost(std::make_unique<Sprite>("")),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    for (const auto &part : available_parts)
    {
        resource_manager.load_texture(part, "images/" + part + ".png");
    }
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
}

void BuildScreen::update(double /*delta_time*/)
{
    auto selected_part_ = part_selector_window->update(available_parts);
    if (selected_part_ != "")
    {
        selected_part = selected_part_;
        ghost->set_texture(selected_part);
        ghost->set_scale(glm::vec2(100, 100));
        ghost->set_origin(ghost->get_center());
        ghost->set_color(cl_white);
    }
}

void BuildScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();

    if (selected_part != "")
    {
        auto cursor_pos = ImGui::GetCursorPos();
        ghost->set_position({cursor_pos.x, cursor_pos.y});
        renderer.draw(*ghost, projection);
    }

    renderer.end();
}
}