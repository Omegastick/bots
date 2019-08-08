#include <memory>
#include <vector>
#include <math.h>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

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
static float lerp(float start, float end, float interpolate)
{
    return start + interpolate * (end - start);
}

float lerp_angle(float start, float end, float interpolate)
{
    float difference = abs(end - start);
    if (difference > M_PI_2)
    {
        if (end > start)
        {
            start += M_PI * 2;
        }
        else
        {
            end += M_PI * 2;
        }
    }
    float value = (start + ((end - start) * interpolate));
    float range = M_PI;
    if (value >= -M_PI && value <= M_PI)
        return value;
    return std::fmod(value + M_PI, range) - M_PI;
}

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

void PlaybackEnv::reset()
{
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

    // If current time is too far behind for some reason, fast forward
    current_tick = std::max(current_tick, static_cast<double>(second_latest_received_tick));

    for (const auto &state : states)
    {
        if (abs(current_tick - state.tick) < 0.0001)
        {
            env->set_state(state);
            return;
        }
    }

    // Lerp start and end states
    // Calculate interpolation value for lerp
    float interpolation = (current_tick - second_latest_received_tick);

    EnvState *start_state;
    EnvState *end_state;

    if (states[0].tick < states[1].tick)
    {
        start_state = &states[0];
        end_state = &states[1];
    }
    else
    {
        start_state = &states[1];
        end_state = &states[0];
    }

    EnvState lerped_state;

    // Agents
    for (unsigned int i = 0; i < start_state->agent_transforms.size(); ++i)
    {
        const b2Transform &start_transform = start_state->agent_transforms[i];
        const b2Transform &end_transform = end_state->agent_transforms[i];
        b2Transform transform;
        transform.p.x = lerp(start_transform.p.x,
                             end_transform.p.x,
                             interpolation);
        transform.p.y = lerp(start_transform.p.y,
                             end_transform.p.y,
                             interpolation);
        transform.q = b2Rot(lerp_angle(start_transform.q.GetAngle(),
                                       end_transform.q.GetAngle(),
                                       interpolation));
        lerped_state.agent_transforms.push_back(transform);
    }

    // Entities
    for (const auto &pair : start_state->entity_states)
    {
        const unsigned int &id = pair.first;
        if (end_state->entity_states.find(id) != end_state->entity_states.end())
        {
            const b2Transform &start_transform = pair.second;
            const b2Transform &end_transform = end_state->entity_states[id];
            b2Transform transform;
            transform.p.x = lerp(start_transform.p.x,
                                 end_transform.p.x,
                                 interpolation);
            transform.p.y = lerp(start_transform.p.y,
                                 end_transform.p.y,
                                 interpolation);
            transform.q = b2Rot(lerp_angle(start_transform.q.GetAngle(),
                                           end_transform.q.GetAngle(),
                                           interpolation));
            lerped_state.entity_states[id] = transform;
        }
    }

    // Update env according to best state
    env->set_state(lerped_state);
    return;
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

    SUBCASE("Body positions are interpolated correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 1}, b2Rot(0)},
                                                  {{2, 3}, b2Rot(1)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, 0});

        agent_transforms = {{{1, 2}, b2Rot(1)},
                            {{3, 4}, b2Rot(0.5)}};
        playback_env.add_new_state({agent_transforms, entity_states, 1});

        playback_env.update(0.05);
        auto agent_0_tranform = env.get_bodies()[0]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(0.5));
        DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(1.5));
        DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(0.5));
        auto agent_1_tranform = env.get_bodies()[1]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(2.5));
        DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(3.5));
        DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(0.75));

        playback_env.update(0.025);
        agent_0_tranform = env.get_bodies()[0]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(0.75));
        DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(1.75));
        DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(0.75));
        agent_1_tranform = env.get_bodies()[1]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(2.75));
        DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(3.75));
        DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(0.625));
    }

    SUBCASE("Body positions are interpolated correctly for ticks after 1")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 1}, b2Rot(0)},
                                                  {{2, 3}, b2Rot(1)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, 3});

        agent_transforms = {{{1, 2}, b2Rot(1)},
                            {{3, 4}, b2Rot(0.5)}};
        playback_env.add_new_state({agent_transforms, entity_states, 4});

        playback_env.update(0.35);
        auto agent_0_tranform = env.get_bodies()[0]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(0.5));
        DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(1.5));
        DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(0.5));
        auto agent_1_tranform = env.get_bodies()[1]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(2.5));
        DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(3.5));
        DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(0.75));

        playback_env.update(0.025);
        agent_0_tranform = env.get_bodies()[0]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(0.75));
        DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(1.75));
        DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(0.75));
        agent_1_tranform = env.get_bodies()[1]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(2.75));
        DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(3.75));
        DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(0.625));
    }

    SUBCASE("Body positions are interpolated past rotation boundaries")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(M_PI - 1)},
                                                  {{0, 0}, b2Rot(-M_PI + 1)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, 0});

        agent_transforms = {{{0, 0}, b2Rot(-M_PI + 1)},
                            {{0, 0}, b2Rot(M_PI - 1)}};
        playback_env.add_new_state({agent_transforms, entity_states, 1});

        playback_env.update(0.05);
        auto agent_0_tranform = env.get_bodies()[0]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(M_PI));
        auto agent_1_tranform = env.get_bodies()[1]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(M_PI));
    }
}
}