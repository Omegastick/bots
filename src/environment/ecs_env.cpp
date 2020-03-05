#include <memory>
#include <queue>

#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

#include "ecs_env.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/components/physics_body.h"
#include "environment/systems/modules/gun_module_system.h"
#include "environment/systems/module_system.h"
#include "environment/systems/physics_system.h"
#include "environment/systems/render_system.h"
#include "environment/systems/trail_system.h"
#include "environment/utils/body_utils.h"
#include "environment/utils/wall_utils.h"
#include "graphics/renderers/renderer.h"
#include "misc/transform.h"

namespace ai
{
EcsEnv::EcsEnv()
{
    init_physics(registry);

    const auto body_entity = make_body(registry);
    auto &body = registry.get<EcsBody>(body_entity);
    body.name = "Steve";
    body.hp = 10;

    const auto gun_module_entity = make_gun_module(registry);
    link_modules(registry, body.base_module, 0, gun_module_entity, 1);
    update_body_fixtures(registry, body_entity);
    auto &physics_body = registry.get<PhysicsBody>(body_entity);
    physics_body.body->SetTransform({9.6f, 5.4f}, glm::radians(-55.f));

    make_wall(registry, {4.f, 0.f}, {2.f, 2.f}, 0.f);
    make_wall(registry, {17.2, 8.8f}, {4.f, 4.f}, 0.f);

    registry.get<Activatable>(gun_module_entity).active = true;
}

EcsEnv::~EcsEnv() {}

void EcsEnv::draw(Renderer &renderer, bool /*lightweight*/)
{
    trail_system(registry);
    render_system(registry, renderer);
    // debug_render_system(registry, renderer);
}

void EcsEnv::forward(double step_length)
{
    physics_system(registry, step_length);
    module_system(registry);
    gun_module_system(registry);
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
    return {};
}
}