#include <memory>

#include <Box2D/Box2D.h>
#include <entt/entt.hpp>

#include "ecs_env.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_world.h"
#include "environment/systems/physics_system.h"
#include "environment/systems/render_system.h"
#include "graphics/renderers/renderer.h"
#include "misc/transform.h"

namespace ai
{
EcsEnv::EcsEnv()
{
    auto world_entity = registry.create();
    auto &world = registry.assign<PhysicsWorld>(world_entity);
    world.world = std::make_unique<b2World>(b2Vec2(0, -1));

    auto entity = registry.create();
    auto &body = registry.assign<PhysicsBody>(entity);
    b2BodyDef body_def;
    body_def.type = b2_dynamicBody;
    body_def.position = {9.6f, 5.4f};
    body.body = world.world->CreateBody(&body_def);
    auto &rectangle = registry.assign<EcsRectangle>(entity);
    rectangle.fill_color = {1.f, 1.f, 1.f, 1.f};
    auto &transform = registry.assign<Transform>(entity);
    transform.set_scale({2, 2});
    transform.set_position({9.6f, 5.4f});
}

EcsEnv::~EcsEnv() {}

void EcsEnv::draw(Renderer &renderer, bool /*lightweight*/)
{
    render_system(registry, renderer);
}

void EcsEnv::forward(double step_length)
{
    physics_system(registry, step_length);
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

void EcsEnv::set_audibility(bool /*visibility*/)
{
}

EcsStepInfo EcsEnv::step(std::vector<torch::Tensor> /*actions*/, double step_length)
{
    physics_system(registry, step_length);
    return {};
}
}