#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>

#include "bullet_contact_handler.h"
#include "environment/components/audio_emitter.h"
#include "environment/components/body.h"
#include "environment/components/bullet.h"
#include "environment/components/distortion_emitter.h"
#include "environment/components/particle_emitter.h"
#include "environment/components/physics_type.h"
#include "environment/utils/bullet_utils.h"
#include "environment/utils/hill_utils.h"
#include "environment/utils/wall_utils.h"
#include "graphics/colors.h"
#include "misc/transform.h"

namespace ai
{
void begin_bullet_contact(entt::registry &registry,
                          entt::entity bullet_entity,
                          entt::entity other_entity,
                          PhysicsType::Type other_type)
{
    if (other_type == PhysicsType::Hill)
    {
        return;
    }

    const auto &transform = registry.get<Transform>(bullet_entity);

    const auto explosion_entity = registry.create();
    const auto distortion_entity = registry.create();
    const auto audio_entity = registry.create();
    if (other_type == PhysicsType::Body)
    {
        registry.emplace<ParticleEmitter>(explosion_entity,
                                          transform.get_position(),
                                          200u,
                                          cl_white,
                                          set_alpha(cl_white, 0),
                                          0.75f,
                                          0.03f,
                                          false);
        registry.emplace<DistortionEmitter>(distortion_entity,
                                            transform.get_position(),
                                            2.f,
                                            0.8f);
        registry.emplace<AudioEmitter>(audio_entity, audio_id_map["hit_body"]);

        registry.get<EcsBody>(other_entity).hp -= registry.get<EcsBullet>(bullet_entity).damage;
    }
    else
    {
        registry.emplace<ParticleEmitter>(explosion_entity,
                                          transform.get_position(),
                                          100u,
                                          cl_white,
                                          set_alpha(cl_white, 0),
                                          0.75f,
                                          0.03f);
        registry.emplace<DistortionEmitter>(distortion_entity,
                                            transform.get_position(),
                                            2.f,
                                            0.1f);
        registry.emplace<AudioEmitter>(audio_entity, audio_id_map["hit_wall"]);
    }

    registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(bullet_entity);
}

TEST_CASE("Bullet contact handler")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    const auto bullet_entity = make_bullet(registry);

    SUBCASE("Explodes on contact with wall")
    {
        const auto other_entity = make_wall(registry, {0.f, 0.f}, {1.f, 1.f}, 0.f);

        begin_bullet_contact(registry, bullet_entity, other_entity, PhysicsType::Wall);

        DOCTEST_CHECK(registry.has<entt::tag<"should_destroy"_hs>>(bullet_entity));
    }

    SUBCASE("Doesn't explode on contact with hill")
    {
        const auto other_entity = make_hill(registry, {0.f, 0.f}, 1.f);

        begin_bullet_contact(registry, bullet_entity, other_entity, PhysicsType::Hill);

        DOCTEST_CHECK(registry.valid(bullet_entity));
    }
}
}