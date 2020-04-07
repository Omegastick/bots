#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <fmt/format.h>
#include <glm/glm.hpp>

#include "thruster_module_system.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/components/modules/module.h"
#include "environment/components/modules/thruster_module.h"
#include "environment/components/particle_emitter.h"
#include "environment/components/physics_body.h"
#include "environment/utils/body_factories.h"
#include "environment/utils/body_utils.h"
#include "misc/transform.h"

namespace ai
{
void thruster_module_system(entt::registry &registry)
{
    const auto view = registry.view<EcsThrusterModule>();
    for (const auto entity : view)
    {
        if (!registry.get<Activatable>(entity).active)
        {
            return;
        }

        const auto &thruster_module = registry.get<EcsThrusterModule>(entity);
        const auto &transform = registry.get<Transform>(entity);
        const auto position = transform.get_position();
        const auto rotation = transform.get_rotation();
        const auto &module = registry.get<EcsModule>(entity);
        auto &physics_body = registry.get<PhysicsBody>(module.body);
        physics_body.body->ApplyForce({-glm::sin(rotation) * thruster_module.force,
                                       glm::cos(rotation) * thruster_module.force},
                                      {position.x, position.y},
                                      true);
    }
}

void thruster_particle_system(entt::registry &registry)
{
    const auto view = registry.view<EcsThrusterModule>();
    for (const auto entity : view)
    {
        if (!registry.get<Activatable>(entity).active)
        {
            return;
        }

        const auto &transform = registry.get<Transform>(entity);
        const auto position = transform.get_position();
        const auto rotation = transform.get_rotation();
        const auto particle_emitter = registry.create();
        registry.emplace<ParticleEmitter>(particle_emitter,
                                          position,
                                          15u,
                                          glm::vec4{1.23f, 0.53f, 0.28f, 1.f},
                                          glm::vec4{0.f, 0.f, 0.f, -0.f},
                                          0.7f,
                                          0.05f,
                                          true,
                                          rotation + glm::radians(180.f));
    }
}

TEST_CASE("Thruster module system")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto body_entity = make_body(registry);
    const auto thruster_module_entity = make_thruster_module(registry);
    const auto &body = registry.get<EcsBody>(body_entity);
    link_modules(registry, body.base_module, 0, thruster_module_entity, 0);
    auto &transform = registry.get<Transform>(thruster_module_entity);
    transform.set_rotation(glm::radians(-90.f));

    SUBCASE("When not active, does nothing")
    {
        thruster_module_system(registry);

        registry.ctx<b2World>().Step(0.1f, 1, 1);
        const auto &physics_body = registry.get<PhysicsBody>(body_entity);
        const auto velocity = physics_body.body->GetLinearVelocity();

        const auto info_string = fmt::format("{{x: {}, y: {}}}", velocity.x, velocity.y);
        INFO(info_string);
        DOCTEST_CHECK(velocity.x == doctest::Approx(0.f));
        DOCTEST_CHECK(velocity.y == doctest::Approx(0.f));
    }

    SUBCASE("When active, thrusts in the pointed direction")
    {
        auto &activatable = registry.get<Activatable>(thruster_module_entity);
        activatable.active = true;

        thruster_module_system(registry);

        registry.ctx<b2World>().Step(0.1f, 1, 1);
        const auto &physics_body = registry.get<PhysicsBody>(body_entity);
        const auto velocity = physics_body.body->GetLinearVelocity();

        const auto info_string = fmt::format("{{x: {}, y: {}}}", velocity.x, velocity.y);
        INFO(info_string);
        DOCTEST_CHECK(velocity.x > 0);
        DOCTEST_CHECK(velocity.y == doctest::Approx(0.f));
    }
}
}