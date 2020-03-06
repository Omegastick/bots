#include <memory>
#include <queue>

#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include "ecs_env.h"
#include "audio/audio_engine.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/components/physics_body.h"
#include "environment/systems/audio_system.h"
#include "environment/systems/clean_up_system.h"
#include "environment/systems/distortion_system.h"
#include "environment/systems/hill_system.h"
#include "environment/systems/modules/gun_module_system.h"
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
{
    init_physics(registry);

    const auto body_entity_1 = make_body(registry);
    auto &body_1 = registry.get<EcsBody>(body_entity_1);
    body_1.name = "Steve";
    body_1.hp = 10;
    const auto gun_module_entity_1 = make_gun_module(registry);
    link_modules(registry, body_1.base_module, 0, gun_module_entity_1, 1);
    update_body_fixtures(registry, body_entity_1);
    auto &physics_body_1 = registry.get<PhysicsBody>(body_entity_1);
    physics_body_1.body->SetTransform({0.f, -15.f}, 0);

    const auto body_entity_2 = make_body(registry);
    auto &body_2 = registry.get<EcsBody>(body_entity_2);
    body_2.name = "Steve";
    body_2.hp = 10;
    const auto gun_module_entity_2 = make_gun_module(registry);
    link_modules(registry, body_2.base_module, 0, gun_module_entity_2, 1);
    update_body_fixtures(registry, body_entity_2);
    auto &physics_body_2 = registry.get<PhysicsBody>(body_entity_2);
    physics_body_2.body->SetTransform({0.f, -6.f}, glm::radians(180.f));
    registry.get<Activatable>(gun_module_entity_2).active = true;

    make_wall(registry, {0.f, -20.f}, {20.f, 0.1f}, 0.f);
    make_wall(registry, {0.f, 20.f}, {20.f, 0.1f}, 0.f);
    make_wall(registry, {-10.f, 0.f}, {0.1f, 40.1f}, 0.f);
    make_wall(registry, {10.f, 0.f}, {0.1f, 40.1f}, 0.f);
    make_wall(registry, {0, -10.f}, {5.f, 0.2f}, 0.f);
    make_wall(registry, {0, 10.f}, {5.f, 0.2f}, 0.f);

    make_hill(registry, {0.f, 0.f}, 3.f);
}

void EcsEnv::draw(Renderer &renderer, IAudioEngine &audio_engine, bool /*lightweight*/)
{
    const double view_height = 50;
    auto view_top = view_height * 0.5;
    auto view_right = view_top * (static_cast<float>(renderer.get_width()) /
                                  static_cast<float>(renderer.get_height()));
    const auto view = glm::ortho(-view_right, view_right, -view_top, view_top);
    renderer.set_view(view);

    trail_system(registry);
    particle_system(registry, renderer);
    distortion_system(registry, renderer);
    render_system(registry, renderer);
    audio_system(registry, audio_engine);
    // debug_render_system(registry, renderer);
}

void EcsEnv::forward(double step_length)
{
    physics_system(registry, step_length);
    clean_up_system(registry);
}

double EcsEnv::get_elapsed_time() const
{
    return 0;
}

bool EcsEnv::is_audible() const
{
    return true;
}

EcsStepInfo EcsEnv::reset()
{
    return {};
}

void EcsEnv::set_audibility(bool /*audibility*/)
{
}

EcsStepInfo EcsEnv::step(std::vector<torch::Tensor> /*actions*/, double step_length)
{
    forward(step_length);
    module_system(registry);
    gun_module_system(registry);
    hill_system(registry);
    return {};
}
}