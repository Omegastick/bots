#include <memory>
#include <queue>

#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "ecs_env.h"
#include "audio/audio_engine.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/particle_emitter.h"
#include "environment/components/physics_body.h"
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
#include "environment/utils/body_utils.h"
#include "environment/utils/hill_utils.h"
#include "environment/utils/wall_utils.h"
#include "graphics/renderers/renderer.h"
#include "misc/transform.h"

namespace ai
{
EcsEnv::EcsEnv()
    : audible(true),
      bodies{entt::null, entt::null}
{
    init_physics(registry);

    const auto body_entity_1 = make_body(registry);
    bodies[0] = body_entity_1;
    auto &body_1 = registry.get<EcsBody>(body_entity_1);
    body_1.name = "Steve";
    body_1.hp = 10;
    body_1.max_hp = 10;
    const auto gun_module_entity_1 = make_gun_module(registry);
    link_modules(registry, body_1.base_module, 0, gun_module_entity_1, 1);
    const auto thruster_module_entity = make_thruster_module(registry);
    link_modules(registry, body_1.base_module, 1, thruster_module_entity, 0);
    update_body_fixtures(registry, body_entity_1);
    auto &physics_body_1 = registry.get<PhysicsBody>(body_entity_1);
    physics_body_1.body->SetTransform({0.f, -15.f}, 0);
    registry.get<Activatable>(thruster_module_entity).active = true;

    const auto body_entity_2 = make_body(registry);
    bodies[1] = body_entity_2;
    auto &body_2 = registry.get<EcsBody>(body_entity_2);
    body_2.name = "Steve";
    body_2.hp = 10;
    body_2.max_hp = 10;
    const auto gun_module_entity_2 = make_gun_module(registry);
    link_modules(registry, body_2.base_module, 0, gun_module_entity_2, 1);
    update_body_fixtures(registry, body_entity_2);
    auto &physics_body_2 = registry.get<PhysicsBody>(body_entity_2);
    physics_body_2.body->SetTransform({0.f, 15.f}, glm::radians(180.f));
    registry.get<Activatable>(gun_module_entity_2).active = true;

    make_wall(registry, {0.f, -20.f}, {20.f, 0.1f}, 0.f);
    make_wall(registry, {0.f, 20.f}, {20.f, 0.1f}, 0.f);
    make_wall(registry, {-10.f, 0.f}, {0.1f, 40.1f}, 0.f);
    make_wall(registry, {10.f, 0.f}, {0.1f, 40.1f}, 0.f);
    make_wall(registry, {0, -10.f}, {5.f, 0.2f}, 0.f);
    make_wall(registry, {0, 10.f}, {5.f, 0.2f}, 0.f);

    make_hill(registry, {0.f, 0.f}, 3.f);

    const auto background_entity = registry.create();
    registry.assign<EcsRectangle>(background_entity, set_alpha(cl_base03, 0.95f));
    auto &background_transform = registry.assign<Transform>(background_entity);
    background_transform.set_scale({20.f, 40.f});
    background_transform.set_z(-5);
}

void EcsEnv::draw(Renderer &renderer, IAudioEngine &audio_engine, bool /*lightweight*/)
{
    const double view_height = 50;
    auto view_top = view_height * 0.5;
    auto view_right = view_top * (static_cast<float>(renderer.get_width()) /
                                  static_cast<float>(renderer.get_height()));
    const auto view = glm::ortho(-view_right, view_right, -view_top, view_top);
    renderer.set_view(view);

    health_bar_system(registry);
    trail_system(registry);
    particle_system(registry, renderer);
    distortion_system(registry, renderer);
    render_system(registry, renderer);
    audio_system(registry, audio_engine);
    debug_render_system(registry, renderer);
}

void EcsEnv::forward(double step_length)
{
    physics_system(registry, step_length);
    module_system(registry);
    thruster_particle_system(registry);
    clean_up_system(registry);
    elapsed_time += step_length;
}

double EcsEnv::get_elapsed_time() const
{
    return elapsed_time;
}

bool EcsEnv::is_audible() const
{
    return audible;
}

EcsStepInfo EcsEnv::reset()
{
    return {};
}

void EcsEnv::set_audibility(bool audibility)
{
    audible = audibility;
}

void EcsEnv::set_body(std::size_t index, const nlohmann::json &body_def)
{
    if (bodies[index] != entt::null)
    {
        destroy_body(registry, bodies[index]);
        clean_up_system(registry);
    }

    const auto body_entity = make_body(registry);
    bodies[index] = body_entity;
}

EcsStepInfo EcsEnv::step(std::vector<torch::Tensor> /*actions*/, double step_length)
{
    forward(step_length);
    gun_module_system(registry);
    thruster_module_system(registry);
    hill_system(registry);

    return {};
}
}