#include <memory>
#include <queue>

#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

#include "ecs_env.h"
#include "environment/components/body.h"
#include "environment/components/physics_body.h"
#include "environment/observers/destroy_body.h"
#include "environment/systems/module_system.h"
#include "environment/systems/physics_system.h"
#include "environment/systems/render_system.h"
#include "environment/utils/body_utils.h"
#include "graphics/renderers/renderer.h"
#include "misc/transform.h"

namespace ai
{
EcsEnv::EcsEnv()
{
    registry.set<b2World>(b2Vec2{0, -1});

    registry.on_destroy<PhysicsBody>().connect<destroy_body>();

    const auto body_entity = create_body(registry);
    auto &body = registry.get<EcsBody>(body_entity);
    body.name = "Steve";
    body.hp = 10;

    const auto module_b_entity = create_base_module(registry);
    link_modules(registry, body.base_module, 0, module_b_entity, 0);
    update_body_fixtures(registry, body_entity);
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