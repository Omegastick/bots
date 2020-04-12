#include <doctest.h>
#include <entt/entt.hpp>

#include "new_frame_system.h"
#include "environment/components/audio_emitter.h"
#include "environment/components/distortion_emitter.h"
#include "environment/components/particle_emitter.h"
#include "environment/components/trail.h"
#include "graphics/render_data.h"

namespace ai
{
void new_frame_system(entt::registry &registry)
{
    // Destroy audio emitters
    const auto audio_emitter_view = registry.view<AudioEmitter>();
    registry.destroy(audio_emitter_view.begin(), audio_emitter_view.end());

    // Destroy distortion emitters
    const auto distortion_emitter_view = registry.view<DistortionEmitter>();
    registry.destroy(distortion_emitter_view.begin(), distortion_emitter_view.end());

    // Destroy lasers
    const auto laser_view = registry.view<entt::tag<"laser"_hs>>();
    registry.destroy(laser_view.begin(), laser_view.end());

    // Destroy particle emitters
    const auto particle_emitter_view = registry.view<ParticleEmitter>();
    registry.destroy(particle_emitter_view.begin(), particle_emitter_view.end());

    // Remove trails
    const auto trail_view = registry.view<Trail>();
    for (const auto &entity : trail_view)
    {
        registry.remove_if_exists<Line>(entity);
    }
}

TEST_CASE("New frame system")
{
    entt::registry registry;
    const auto entity = registry.create();

    SUBCASE("Destroys audio emitters")
    {
        registry.emplace<AudioEmitter>(entity);
        new_frame_system(registry);
        DOCTEST_CHECK(!registry.valid(entity));
    }

    SUBCASE("Destroys distortion emitters")
    {
        registry.emplace<DistortionEmitter>(entity);
        new_frame_system(registry);
        DOCTEST_CHECK(!registry.valid(entity));
    }

    SUBCASE("Destroys lasers")
    {
        registry.emplace<entt::tag<"laser"_hs>>(entity);
        new_frame_system(registry);
        DOCTEST_CHECK(!registry.valid(entity));
    }

    SUBCASE("Destroys particle emitters")
    {
        registry.emplace<ParticleEmitter>(entity);
        new_frame_system(registry);
        DOCTEST_CHECK(!registry.valid(entity));
    }

    SUBCASE("Removes trails")
    {
        registry.emplace<Trail>(entity);
        registry.emplace<Line>(entity);
        new_frame_system(registry);
        DOCTEST_CHECK(registry.valid(entity));
        DOCTEST_CHECK(!registry.has<Line>(entity));
    }
}
}