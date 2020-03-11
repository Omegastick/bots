#include <limits>

#include <doctest.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "trail_system.h"
#include "environment/components/trail.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "misc/transform.h"

namespace ai
{
void trail_system(entt::registry &registry)
{
    registry.view<Trail, Transform>().each([&](const auto entity, auto &trail, auto &transform) {
        const auto current_position = transform.get_position();
        if (trail.previous_positions[0][0] == -std::numeric_limits<float>::infinity())
        {
            trail.previous_positions = {current_position, current_position, current_position};
        }
        else
        {
            trail.previous_positions[2] = trail.previous_positions[1];
            trail.previous_positions[1] = trail.previous_positions[0];
            trail.previous_positions[0] = current_position;
        }

        registry.assign_or_replace<Line>(entity,
                                         trail.previous_positions[0],
                                         trail.previous_positions[2],
                                         cl_white,
                                         trail.width);
    });
}

TEST_CASE("trail_system")
{
    entt::registry registry;

    const auto entity = registry.create();
    registry.assign<Trail>(entity);
    registry.assign<Transform>(entity);

    SUBCASE("Creates a Line component")
    {
        trail_system(registry);

        DOCTEST_CHECK(registry.has<Line>(entity));
    }

    SUBCASE("When a trail is first created, previous positions are set to the entities current position")
    {
        auto &transform = registry.get<Transform>(entity);
        trail_system(registry);
        transform.set_position({1.f, 1.f});
        trail_system(registry);

        const auto &trail = registry.get<Trail>(entity);
        DOCTEST_CHECK(trail.previous_positions[0] == glm::vec2{1.f, 1.f});
        DOCTEST_CHECK(trail.previous_positions[1] == glm::vec2{0.f, 0.f});
        DOCTEST_CHECK(trail.previous_positions[2] == glm::vec2{0.f, 0.f});
    }

    SUBCASE("Trails follow their entity")
    {
        auto &transform = registry.get<Transform>(entity);
        transform.set_position({1.f, 2.f});
        trail_system(registry);
        transform.set_position({2.f, 3.f});
        trail_system(registry);
        transform.set_position({3.f, 4.f});
        trail_system(registry);
        transform.set_position({4.f, 5.f});
        trail_system(registry);

        const auto &trail = registry.get<Trail>(entity);
        DOCTEST_CHECK(trail.previous_positions[0] == glm::vec2{4.f, 5.f});
        DOCTEST_CHECK(trail.previous_positions[1] == glm::vec2{3.f, 4.f});
        DOCTEST_CHECK(trail.previous_positions[2] == glm::vec2{2.f, 3.f});
    }
}
}