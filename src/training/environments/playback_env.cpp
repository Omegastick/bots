#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>

#include "playback_env.h"
#include "graphics/render_data.h"
#include "misc/random.h"
#include "training/bodies/test_body.h"
#include "training/entities/ientity.h"
#include "training/environments/koth_env.h"
#include "training/environments/ienvironment.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
PlaybackEnv::PlaybackEnv(std::unique_ptr<IEnvironment> env, double tick_length)
    : current_tick(0),
      env(std::move(env)),
      tick_length(tick_length)
{
}

void PlaybackEnv::add_new_state(EnvState /*state*/) {}
RenderData PlaybackEnv::get_render_data(bool /*lightweight*/) {}
void PlaybackEnv::update(double /*delta_time*/) {}

TEST_CASE("PlaybackEnv")
{
    auto b2_world = std::make_unique<b2World>(b2Vec2{0, 0});
    auto rng = std::make_unique<Random>(0);
    TestBodyFactory body_factory(*rng);
    std::vector<std::unique_ptr<Body>> bodies;
    bodies.push_back(body_factory.make(*b2_world, *rng));
    bodies.push_back(body_factory.make(*b2_world, *rng));
    KothEnvFactory env_factory(100);
    auto env = env_factory.make(std::move(rng),
                                std::move(b2_world),
                                std::move(bodies),
                                RewardConfig());
    PlaybackEnv playback_env(std::move(env), 0.1);

    SUBCASE("New bullets are added correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::vector<EntityState> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, 0});

        entity_states = {{0, {{0, 0}, b2Rot(0)}},
                         {1, {{1, 1}, b2Rot(1)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 1});

        playback_env.update(0);
        DOCTEST_CHECK(env.get_entities().size() == 0);

        playback_env.update(0.1);
        DOCTEST_CHECK(env.get_entities().size() == 2);
    }

    SUBCASE("Bullets are moved correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::vector<EntityState> entity_states{{0, {{0, 0}, b2Rot(0)}},
                                               {1, {{1, 1}, b2Rot(1)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 0});

        entity_states = {{0, {{0.5, 0.5}, b2Rot(0.5)}},
                         {1, {{-1, 0}, b2Rot(2)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 1});

        playback_env.update(0);
        auto bullet_0_tranform = env.get_entities()[0]->get_transform();
        DOCTEST_CHECK(bullet_0_tranform.p.x == doctest::Approx(0));
        DOCTEST_CHECK(bullet_0_tranform.p.y == doctest::Approx(0));
        DOCTEST_CHECK(bullet_0_tranform.q.GetAngle() == doctest::Approx(0));
        auto bullet_1_tranform = env.get_entities()[1]->get_transform();
        DOCTEST_CHECK(bullet_1_tranform.p.x == doctest::Approx(1));
        DOCTEST_CHECK(bullet_1_tranform.p.y == doctest::Approx(1));
        DOCTEST_CHECK(bullet_1_tranform.q.GetAngle() == doctest::Approx(1));

        playback_env.update(0.1);
        bullet_0_tranform = env.get_entities()[0]->get_transform();
        DOCTEST_CHECK(bullet_0_tranform.p.x == doctest::Approx(0.5));
        DOCTEST_CHECK(bullet_0_tranform.p.y == doctest::Approx(0.5));
        DOCTEST_CHECK(bullet_0_tranform.q.GetAngle() == doctest::Approx(0.5));
        bullet_1_tranform = env.get_entities()[1]->get_transform();
        DOCTEST_CHECK(bullet_1_tranform.p.x == doctest::Approx(-1));
        DOCTEST_CHECK(bullet_1_tranform.p.y == doctest::Approx(0));
        DOCTEST_CHECK(bullet_1_tranform.q.GetAngle() == doctest::Approx(2));
    }

    SUBCASE("Bullets are removed correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::vector<EntityState> entity_states{{0, {{0, 0}, b2Rot(0)}},
                                               {1, {{1, 1}, b2Rot(1)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 0});

        entity_states = {};
        playback_env.add_new_state({agent_transforms, entity_states, 1});

        playback_env.update(0);
        DOCTEST_CHECK(env.get_entities().size() == 2);

        playback_env.update(0.1);
        DOCTEST_CHECK(env.get_entities().size() == 0);
    }

    SUBCASE("Bodies are moved correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 1}, b2Rot(0)},
                                                  {{2, 3}, b2Rot(1)}};
        std::vector<EntityState> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, 0});

        agent_transforms = {{{1, 2}, b2Rot(1)},
                            {{3, 4}, b2Rot(0.5)}};
        playback_env.add_new_state({agent_transforms, entity_states, 1});

        playback_env.update(0);
        auto agent_0_tranform = env.get_bodies()[0]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(0));
        DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(1));
        DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(0));
        auto agent_1_tranform = env.get_bodies()[1]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(2));
        DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(3));
        DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(1));

        playback_env.update(0.1);
        agent_0_tranform = env.get_entities()[0]->get_transform();
        DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(1));
        DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(2));
        DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(0));
        agent_1_tranform = env.get_entities()[1]->get_transform();
        DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(3));
        DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(4));
        DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(0.5));
    }

    SUBCASE("Correct tick is chosen when multiple are available")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::vector<EntityState> entity_states{{0, {{0, 0}, b2Rot(0)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 0});
        entity_states = {{0, {{1, 1}, b2Rot(1)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 1});
        entity_states = {{0, {{2, 2}, b2Rot(2)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 2});
        entity_states = {{0, {{3, 3}, b2Rot(3)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 3});
        entity_states = {{0, {{4, 4}, b2Rot(4)}}};
        playback_env.add_new_state({agent_transforms, entity_states, 4});

        playback_env.update(0.3);
        auto bullet_tranform = env.get_entities()[0]->get_transform();
        DOCTEST_CHECK(bullet_tranform.p.x == doctest::Approx(3));
    }
}
}