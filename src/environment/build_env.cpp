#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "build_env.h"
#include "audio/audio_engine.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/health_bar.h"
#include "environment/components/modules/module.h"
#include "environment/components/name.h"
#include "environment/serialization/serialize_body.h"
#include "environment/systems/audio_system.h"
#include "environment/systems/clean_up_system.h"
#include "environment/systems/distortion_system.h"
#include "environment/systems/health_bar_system.h"
#include "environment/systems/hill_system.h"
#include "environment/systems/modules/gun_module_system.h"
#include "environment/systems/modules/thruster_module_system.h"
#include "environment/systems/module_system.h"
#include "environment/systems/particle_system.h"
#include "environment/systems/physics_system.h"
#include "environment/systems/render_system.h"
#include "environment/systems/trail_system.h"
#include "environment/utils/body_factories.h"
#include "environment/utils/body_utils.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "misc/transform.h"

namespace ai
{
constexpr float snap_distance = 1.f;

BuildEnv::BuildEnv()
    : cursor_entity(entt::null)
{
    init_physics(registry);

    body_entity = make_body(registry);
    const auto &health_bar = registry.get<HealthBar>(body_entity);
    registry.destroy(health_bar.background);
    registry.destroy(health_bar.foreground);
}

void BuildEnv::draw(Renderer &renderer, IAudioEngine &audio_engine)
{
    trail_system(registry);
    particle_system(registry, renderer);
    render_system(registry, renderer);
    audio_system(registry, audio_engine);
    debug_render_system(registry, renderer);
}

void BuildEnv::forward(double step_length)
{
    physics_system(registry, step_length);
    module_system(registry);
    thruster_particle_system(registry);
    gun_module_system(registry);
    thruster_module_system(registry);
    hill_system(registry);
    clean_up_system(registry);
}

entt::entity BuildEnv::create_module(const std::string &type)
{
    return make_module(registry, type);
}

entt::entity BuildEnv::select_module(glm::vec2 position)
{
    const auto entity = get_module_at_point(registry, position);
    select_module(entity);
    return entity;
}

void BuildEnv::select_module(entt::entity module_entity)
{
    if (cursor_entity == entt::null)
    {
        cursor_entity = registry.create();
        auto &transform = registry.assign<Transform>(cursor_entity);
        transform.set_z(-2);
        registry.assign<EcsRectangle>(cursor_entity, 0.1f);
        registry.assign<Color>(cursor_entity);
    }

    auto &cursor_transform = registry.get<Transform>(cursor_entity);
    if (!registry.valid(module_entity))
    {
        cursor_transform.set_position({100000.f, 100000.f});
        return;
    }

    auto &module_transform = registry.get<Transform>(module_entity);
    cursor_transform.set_position(module_transform.get_position());
    cursor_transform.set_scale(module_transform.get_scale() + glm::vec2{0.2f, 0.2f});
    cursor_transform.set_rotation(module_transform.get_rotation());
}

void BuildEnv::delete_module(entt::entity module_entity)
{
    destroy_module(registry, module_entity);
    auto &module = registry.get<EcsModule>(module_entity);
    if (module.body != entt::null)
    {
        update_body_fixtures(registry, body_entity);
    }
}

void BuildEnv::move_module(entt::entity module_entity, glm::vec2 position, float rotation)
{
    auto &transform = registry.get<Transform>(module_entity);
    transform.set_position(position);
    transform.set_rotation(rotation);
}

void BuildEnv::snap_module(entt::entity module_entity)
{
    const auto nearest_links = find_nearest_link(registry, module_entity);
    if (nearest_links.distance < snap_distance)
    {
        snap_modules(registry,
                     nearest_links.module_a,
                     nearest_links.link_a,
                     module_entity,
                     nearest_links.link_b);
    }
}

bool BuildEnv::link_module(entt::entity module_entity)
{
    const auto nearest_links = find_nearest_link(registry, module_entity);
    if (nearest_links.distance < snap_distance)
    {
        link_modules(registry,
                     nearest_links.link_a,
                     nearest_links.link_b);
        apply_color_scheme(registry, body_entity);
        return true;
    }
    return false;
}

ColorScheme BuildEnv::get_color_scheme() const
{
    return registry.get<ColorScheme>(body_entity);
}

void BuildEnv::set_color_scheme(const ColorScheme &color_scheme)
{
    registry.assign_or_replace<ColorScheme>(body_entity, color_scheme);
    apply_color_scheme(registry, body_entity);
}

std::string BuildEnv::get_name() const
{
    return registry.get<Name>(body_entity).name;
}

void BuildEnv::set_name(const std::string &name)
{
    registry.get<Name>(body_entity).name = name;
}

nlohmann::json BuildEnv::serialize_body() const
{
    return ai::serialize_body(registry, body_entity);
}
}