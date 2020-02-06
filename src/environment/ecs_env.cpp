#include <memory>

#include <Box2D/Box2D.h>
#include <entt/entt.hpp>

#include "ecs_env.h"
#include "environment/components/modules/base_module.h"
#include "environment/components/modules/module.h"
#include "environment/components/body.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_world.h"
#include "environment/observers/destroy_body.h"
#include "environment/systems/module_system.h"
#include "environment/systems/physics_system.h"
#include "environment/systems/render_system.h"
#include "graphics/renderers/renderer.h"
#include "misc/transform.h"

namespace ai
{
entt::entity create_base_module(entt::registry &registry)
{
    const auto entity = registry.create();
    registry.assign<EcsModule>(entity);
    registry.assign<EcsBaseModule>(entity);
    registry.assign<Transform>(entity);

    registry.assign<EcsRectangle>(entity,
                                  glm::vec4{0.5f, 0.5f, 0.5f, 0.5f},
                                  cl_white,
                                  0.1f);
    registry.assign<EcsCircle>(entity,
                               0.2f,
                               glm::vec4{0, 0, 0, 0},
                               cl_white,
                               0.1f);

    return entity;
}

entt::entity create_body(entt::registry &registry)
{
    const auto entity = registry.create();
    auto &body = registry.assign<EcsBody>(entity);
    registry.assign<Transform>(entity);

    auto &physics_body = registry.assign<PhysicsBody>(entity);
    b2BodyDef body_def;
    body_def.type = b2_dynamicBody;
    body_def.position = {9.6f, 5.4f};
    physics_body.body = registry.ctx<b2World>().CreateBody(&body_def);

    const auto base_module_entity = create_base_module(registry);
    body.base_module = base_module_entity;

    return entity;
}

EcsEnv::EcsEnv()
{
    registry.set<b2World>(b2Vec2{0, -1});

    registry.on_destroy<PhysicsBody>().connect<destroy_body>();

    const auto body_entity = create_body(registry);
    auto &body = registry.get<EcsBody>(body_entity);
    body.name = "Steve";
    body.hp = 10;
}

EcsEnv::~EcsEnv() {}

void EcsEnv::draw(Renderer &renderer, bool /*lightweight*/)
{
    render_system(registry, renderer);
}

void EcsEnv::forward(double step_length)
{
    physics_system(registry, step_length);
    module_system(registry);
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