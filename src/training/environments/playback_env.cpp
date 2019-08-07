#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>

#include "playback_env.h"
#include "graphics/render_data.h"
#include "misc/random.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"
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

void PlaybackEnv::add_new_state(EnvState state)
{
    states.push_back(state);
}

RenderData PlaybackEnv::get_render_data(bool lightweight)
{
    return env->get_render_data(lightweight);
}

void PlaybackEnv::reset() {
    env->reset();
}

void PlaybackEnv::set_bodies(const std::vector<nlohmann::json> &body_specs)
{
    for (unsigned int i = 0; i < body_specs.size(); ++i)
    {
        env->get_bodies()[i]->load_json(body_specs[i]);
    }
}

void PlaybackEnv::update(double delta_time)
{
    // Only update if we have at least two states to work from
    if (states.size() < 2)
    {
        return;
    }

    current_tick += delta_time / tick_length;

    int latest_received_tick = -1;
    int second_latest_received_tick = -1;

    // Loop through all received ticks two find latest two
    for (const auto &state : states)
    {
        if (state.tick > latest_received_tick)
        {
            second_latest_received_tick = latest_received_tick;
            latest_received_tick = state.tick;
        }
        else if (state.tick > second_latest_received_tick)
        {
            second_latest_received_tick = state.tick;
        }
    }

    // Delete all older states
    for (auto state_iterator = states.begin(); state_iterator < states.end();)
    {
        if (state_iterator->tick < second_latest_received_tick)
        {
            state_iterator = states.erase(state_iterator);
        }
        else
        {
            ++state_iterator;
        }
    }

    // TODO: Replace the following aliasing logic with interpolation
    EnvState *chosen_state;
    if (abs(states[0].tick - current_tick) < abs(states[1].tick - current_tick))
    {
        chosen_state = &states[0];
    }
    else
    {
        chosen_state = &states[1];
    }

    // Update env according to best state
    env->set_state(*chosen_state);
}

TEST_CASE("PlaybackEnv")
{
    Random rng(0);
    TestBodyFactory body_factory(rng);
    KothEnvFactory env_factory(100, body_factory);
    auto env = env_factory.make();
    PlaybackEnv playback_env(std::move(env), 0.1);

    SUBCASE("New bullets are added correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
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
        std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}},
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
        std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}},
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
        std::unordered_map<unsigned int, b2Transform> entity_states{};
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
        agent_0_tranform = env.get_bodies()[0]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(1));
        DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(2));
        DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(1));
        agent_1_tranform = env.get_bodies()[1]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(3));
        DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(4));
        DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(0.5));
    }

    SUBCASE("Correct tick is chosen when multiple are available")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}}};
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