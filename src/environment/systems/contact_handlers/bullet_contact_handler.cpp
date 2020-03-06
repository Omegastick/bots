#include <entt/entt.hpp>

#include "bullet_contact_handler.h"
#include "environment/components/audio_emitter.h"
#include "environment/components/body.h"
#include "environment/components/bullet.h"
#include "environment/components/distortion_emitter.h"
#include "environment/components/particle_emitter.h"
#include "environment/components/physics_type.h"
#include "graphics/colors.h"
#include "misc/transform.h"

namespace ai
{
void begin_bullet_contact(entt::registry &registry,
                          entt::entity bullet_entity,
                          entt::entity other_entity,
                          PhysicsType::Type other_type)
{
    const auto &transform = registry.get<Transform>(bullet_entity);

    const auto explosion_entity = registry.create();
    const auto distortion_entity = registry.create();
    const auto audio_entity = registry.create();
    if (other_type == PhysicsType::Body)
    {
        registry.assign<ParticleEmitter>(explosion_entity,
                                         transform.get_position(),
                                         200u,
                                         cl_white,
                                         set_alpha(cl_white, 0),
                                         0.75f,
                                         0.03f,
                                         false);
        registry.assign<DistortionEmitter>(distortion_entity,
                                           transform.get_position(),
                                           2.f,
                                           0.8f);
        registry.assign<AudioEmitter>(audio_entity, audio_id_map["hit_body"]);

        registry.get<EcsBody>(other_entity).hp -= registry.get<EcsBullet>(bullet_entity).damage;
    }
    else
    {
        registry.assign<ParticleEmitter>(explosion_entity,
                                         transform.get_position(),
                                         100u,
                                         cl_white,
                                         set_alpha(cl_white, 0),
                                         0.75f,
                                         0.03f,
                                         false);
        registry.assign<DistortionEmitter>(distortion_entity,
                                           transform.get_position(),
                                           2.f,
                                           0.1f);
        registry.assign<AudioEmitter>(audio_entity, audio_id_map["hit_wall"]);
    }

    registry.destroy(bullet_entity);
}
}