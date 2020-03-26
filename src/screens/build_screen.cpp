#include <memory>
#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "build_screen.h"
#include "audio/audio_engine.h"
#include "environment/build_env.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "graphics/render_data.h"
#include "misc/io.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "misc/utilities.h"
#include "ui/back_button.h"
#include "ui/build_screen/part_selector_window.h"
#include "ui/build_screen/unlock_parts_window.h"

namespace ai
{
BuildScreen::BuildScreen(BuildEnv &&build_env,
                         std::unique_ptr<ColorSchemeWindow> color_scheme_window,
                         std::unique_ptr<PartSelectorWindow> part_selector_window,
                         std::unique_ptr<SaveBodyWindow> save_body_window,
                         std::unique_ptr<UnlockPartsWindow> unlock_parts_window,
                         IAudioEngine &audio_engine,
                         IModuleFactory &module_factory,
                         ResourceManager &resource_manager,
                         ScreenManager &screen_manager,
                         IO &io)
    : audio_engine(audio_engine),
      build_env(std::move(build_env)),
      color_scheme_window(std::move(color_scheme_window)),
      current_rotation(0),
      module_factory(module_factory),
      screen_manager(screen_manager),
      show_unlock_parts_window(false),
      io(io),
      part_detail_window(io),
      part_selector_window(std::move(part_selector_window)),
      b2_world(b2Vec2(0, 0)),
      save_body_window(std::move(save_body_window)),
      module_to_place(entt::null),
      selected_module(entt::null),
      unlock_parts_window(std::move(unlock_parts_window))

{
    resource_manager.load_texture("square", "images/square.png");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);
}

void BuildScreen::update(double delta_time)
{
    const auto part_selector_output = part_selector_window->update(module_to_place_name,
                                                                   show_unlock_parts_window);
    if (part_selector_output != "")
    {
        if (module_to_place != entt::null)
        {
            build_env.delete_module(module_to_place);
        }
        module_to_place = build_env.create_module(part_selector_output);
        selected_module = module_to_place;
        module_to_place_name = module_to_place_name;
        ImGui::SetWindowFocus(nullptr);
    }

    if (module_to_place != entt::null)
    {
        const auto cursor_world_position = screen_to_world_space(io.get_cursor_position(),
                                                                 io.get_resolution(),
                                                                 view);
        build_env.move_module(module_to_place, cursor_world_position, current_rotation);
        build_env.snap_module(module_to_place);
    }

    if (io.get_left_click())
    {
        if (module_to_place == entt::null)
        {
            const auto cursor_world_position = screen_to_world_space(io.get_cursor_position(),
                                                                     io.get_resolution(),
                                                                     view);
            selected_module = build_env.select_module(cursor_world_position);
            module_to_place_name = "";
        }
        else
        {
            if (!build_env.link_module(module_to_place))
            {
                build_env.delete_module(module_to_place);
                selected_module = entt::null;
            }
            module_to_place_name = "";
            build_env.select_module(selected_module);
        }
        module_to_place = entt::null;
    }

    if (io.get_key_pressed_this_frame(GLFW_KEY_Q))
    {
        current_rotation += glm::radians(90.f);
    }
    else if (io.get_key_pressed_this_frame(GLFW_KEY_E))
    {
        current_rotation -= glm::radians(90.f);
    }

    if (selected_module != entt::null && io.get_key_pressed_this_frame(GLFW_KEY_DELETE))
    {
        build_env.delete_module(selected_module);
        selected_module = entt::null;
        module_to_place = entt::null;
    }

    // part_detail_window.select_part(selected_module.get());

    auto color_scheme = build_env.get_color_scheme();
    if (color_scheme_window->update(color_scheme))
    {
        build_env.set_color_scheme(color_scheme);
    }

    // part_detail_window.update();
    save_body_window->update(build_env);
    const auto part_bought = unlock_parts_window->update(show_unlock_parts_window);
    if (part_bought)
    {
        part_selector_window->refresh_parts();
    }

    build_env.forward(delta_time);

    auto resolution = io.get_resolution();
    back_button(screen_manager, resolution);
}

void BuildScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    const double view_height = 25;
    auto view_top = view_height * 0.5;
    glm::vec2 resolution = io.get_resolutionf();
    auto view_right = view_top * (resolution.x / resolution.y);
    view = glm::ortho(-view_right, view_right, -view_top, view_top);
    renderer.set_view(view);

    build_env.draw(renderer, audio_engine);

    // body_builder.set_view(projection);

    // if (module_to_place != nullptr)
    // {
    //     module_to_place->get_transform().p = {cursor_world_position.x, cursor_world_position.y};
    //     module_to_place->get_transform().q = b2Rot(glm::radians(current_rotation * 90.f));

    //     auto nearest_link_result = body_builder.get_nearest_module_link_to_module(*module_to_place);
    //     if (nearest_link_result.nearest_link != nullptr && nearest_link_result.distance < 1)
    //     {
    //         nearest_link_result.origin_link->snap_to_other(*nearest_link_result.nearest_link);
    //     }

    //     module_to_place->draw(renderer);
    // }

    // body_builder.draw(renderer);
}

BuildScreenFactory::BuildScreenFactory(Animator &animator,
                                       IAudioEngine &audio_engine,
                                       CredentialsManager &credentials_manager,
                                       IHttpClient &http_client,
                                       IModuleFactory &module_factory,
                                       ModuleTextureStore &module_texture_store,
                                       ResourceManager &resource_manager,
                                       ScreenManager &screen_manager,
                                       IO &io)
    : animator(animator),
      audio_engine(audio_engine),
      credentials_manager(credentials_manager),
      http_client(http_client),
      module_factory(module_factory),
      module_texture_store(module_texture_store),
      resource_manager(resource_manager),
      screen_manager(screen_manager),
      io(io) {}

std::shared_ptr<IScreen> BuildScreenFactory::make()
{
    return std::make_shared<BuildScreen>(BuildEnv(),
                                         std::make_unique<ColorSchemeWindow>(io),
                                         std::make_unique<PartSelectorWindow>(
                                             credentials_manager,
                                             http_client,
                                             io,
                                             module_texture_store,
                                             resource_manager),
                                         std::make_unique<SaveBodyWindow>(animator, io),
                                         std::make_unique<UnlockPartsWindow>(
                                             credentials_manager,
                                             http_client,
                                             io,
                                             module_texture_store,
                                             resource_manager),
                                         audio_engine,
                                         module_factory,
                                         resource_manager,
                                         screen_manager,
                                         io);
}
}