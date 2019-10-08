#include <memory>
#include <vector>
#include <math.h>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <glm/gtc/constants.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "playback_env.h"
#include "graphics/render_data.h"
#include "misc/random.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"
#include "training/entities/ientity.h"
#include "training/events/entity_destroyed.h"
#include "training/events/ievent.h"
#include "training/environments/koth_env.h"
#include "training/environments/ienvironment.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
const float pi = glm::pi<float>();

static float lerp(float start, float end, float interpolate)
{
    return start + interpolate * (end - start);
}

float lerp_angle(float start, float end, float interpolate)
{
    float difference = abs(end - start);
    if (difference > pi / 2.f)
    {
        if (end > start)
        {
            start += pi * 2;
        }
        else
        {
            end += pi * 2;
        }
    }
    float value = start + ((end - start) * interpolate);
    float range = pi;
    if (value >= -pi && value <= pi)
        return value;
    return std::fmod(value + pi, range) - pi;
}

PlaybackEnv::PlaybackEnv(std::unique_ptr<IEnvironment> env, double tick_length)
    : current_tick(0),
      env(std::move(env)),
      tick_length(tick_length)
{
}

void PlaybackEnv::add_events(std::vector<std::unique_ptr<IEvent>> events)
{
    for (auto &event : events)
    {
        this->events.push_back(std::move(event));
    }
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

    EnvState *oldest_relevant_state = nullptr;

    // Loop through all received to find the oldest relevant tick
    for (auto &state : states)
    {
        if (oldest_relevant_state == nullptr ||
            (state.tick > oldest_relevant_state->tick && state.tick <= current_tick))
        {
            oldest_relevant_state = &state;
        }
    }

    // Delete all older states
    int oldest_relevant_tick = oldest_relevant_state->tick;
    for (auto iter = states.begin(); iter < states.end();)
    {
        if (iter->tick < oldest_relevant_tick && states.size() > 2)
        {
            iter = states.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    // The pointer to the oldest relevant state will have moved, so we find it again
    oldest_relevant_state = nullptr;
    for (auto &state : states)
    {
        if (oldest_relevant_state == nullptr ||
            (state.tick > oldest_relevant_state->tick && state.tick <= current_tick))
        {
            oldest_relevant_state = &state;
        }
    }

    // Pick an end state to interpolate to
    EnvState *start_state = nullptr;
    EnvState *end_state = nullptr;
    if (states.size() > 2)
    {
        start_state = oldest_relevant_state;
        for (auto &state : states)
        {
            if ((end_state == nullptr || state.tick < end_state->tick) && &state != start_state)
            {
                end_state = &state;
            }
        }
    }
    else
    {
        // There are only two states to choose from
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
    }

    current_tick = std::min(current_tick, static_cast<double>(end_state->tick));

    // Lerp start and end states
    // Calculate interpolation value for lerp
    float interpolation = current_tick - start_state->tick;

    EnvState lerped_state;

    if (current_tick < end_state->tick)
    {
        lerped_state.hps = start_state->hps;
        lerped_state.scores = start_state->scores;
    }
    else
    {
        lerped_state.hps = end_state->hps;
        lerped_state.scores = end_state->scores;
    }

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
        const b2Transform &start_transform = pair.second;
        // Entities still existing at the end of the tick
        if (end_state->entity_states.find(id) != end_state->entity_states.end())
        {
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
        // Entities destroyed during the tick
        else
        {
            auto event_iter = std::find_if(events.begin(), events.end(),
                                           [&](const std::unique_ptr<IEvent> &event) {
                                               if (event->type == EventTypes::EntityDestroyed)
                                               {
                                                   return static_cast<EntityDestroyed *>(event.get())->get_id() == id;
                                               }
                                               return false;
                                           });
            if (event_iter == events.end())
            {
                continue;
            }
            auto &event = static_cast<EntityDestroyed &>(*event_iter->get());
            double event_tick = event.get_time() / tick_length;

            b2Transform end_transform(b2Vec2(std::get<0>(event.get_transform()),
                                             std::get<1>(event.get_transform())),
                                      b2Rot(std::get<2>(event.get_transform())));

            // Calclulate interpolation for this entity
            double event_tick_interval = event_tick - start_state->tick;
            float temp_interpolation = (current_tick - start_state->tick) / event_tick_interval;
            b2Transform transform;
            transform.p.x = lerp(start_transform.p.x,
                                 end_transform.p.x,
                                 temp_interpolation);
            transform.p.y = lerp(start_transform.p.y,
                                 end_transform.p.y,
                                 temp_interpolation);
            transform.q = b2Rot(lerp_angle(start_transform.q.GetAngle(),
                                           end_transform.q.GetAngle(),
                                           temp_interpolation));
            lerped_state.entity_states[id] = transform;
        }
    }

    // Update env according to best state
    env->set_state(lerped_state);

    // Set elapsed time in env
    env->set_elapsed_time(current_tick * tick_length);

    // Trigger events
    for (auto iter = events.begin(); iter != events.end();)
    {
        auto &event = *iter;
        if (event->get_time() / tick_length <= current_tick)
        {
            event->trigger(*env);
            iter = events.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

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
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        entity_states = {{0, {{0, 0}, b2Rot(0)}},
                         {1, {{1, 1}, b2Rot(1)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 2});

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
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        entity_states = {{0, {{0.5, 0.5}, b2Rot(0.5)}},
                         {1, {{-1, 0}, b2Rot(2)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

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

    SUBCASE("Bodies are moved correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 1}, b2Rot(0)},
                                                  {{2, 3}, b2Rot(1)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        agent_transforms = {{{1, 2}, b2Rot(1)},
                            {{3, 4}, b2Rot(0.5)}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

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
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});
        entity_states = {{0, {{1, 1}, b2Rot(1)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});
        entity_states = {{0, {{2, 2}, b2Rot(2)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 2});
        entity_states = {{0, {{3, 3}, b2Rot(3)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 3});
        entity_states = {{0, {{4, 4}, b2Rot(4)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 4});

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
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        agent_transforms = {{{1, 2}, b2Rot(1)},
                            {{3, 4}, b2Rot(0.5)}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

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
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 3});

        agent_transforms = {{{1, 2}, b2Rot(1)},
                            {{3, 4}, b2Rot(0.5)}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 4});

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

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(pi - 1)},
                                                  {{0, 0}, b2Rot(-pi + 1)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        agent_transforms = {{{0, 0}, b2Rot(-pi + 1)},
                            {{0, 0}, b2Rot(pi - 1)}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

        playback_env.update(0.05);
        auto agent_0_tranform = env.get_bodies()[0]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK_UNARY(agent_0_tranform.q.GetAngle() == doctest::Approx(pi) ||
                            agent_0_tranform.q.GetAngle() == doctest::Approx(-pi));
        auto agent_1_tranform = env.get_bodies()[1]->get_rigid_body().body->GetTransform();
        DOCTEST_CHECK_UNARY(agent_1_tranform.q.GetAngle() == doctest::Approx(pi) ||
                            agent_1_tranform.q.GetAngle() == doctest::Approx(-pi));
    }

    SUBCASE("Events are triggered at the right time")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

        std::vector<std::unique_ptr<IEvent>> events;
        events.push_back(std::make_unique<EntityDestroyed>(0, 0.1, Transform{1, 2, 3}));
        playback_env.add_events(std::move(events));

        playback_env.update(0.05);
        auto render_data = env.get_render_data();
        DOCTEST_CHECK(render_data.particles.size() == 0);

        playback_env.update(0.1);
        render_data = env.get_render_data();
        DOCTEST_CHECK(render_data.particles.size() > 0);
    }

    SUBCASE("Bullets destroyed partway through a tick continue moving until destroyed")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        entity_states = {};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

        std::vector<std::unique_ptr<IEvent>> events;
        events.push_back(std::make_unique<EntityDestroyed>(0, 0.05, Transform{1, 2, 1}));
        playback_env.add_events(std::move(events));

        playback_env.update(0.025);
        auto bullet_tranform = env.get_entities()[0]->get_transform();
        DOCTEST_CHECK(bullet_tranform.p.x == doctest::Approx(0.5));
        DOCTEST_CHECK(bullet_tranform.p.y == doctest::Approx(1));
        DOCTEST_CHECK(bullet_tranform.q.GetAngle() == doctest::Approx(0.5));

        playback_env.update(0.025);
        DOCTEST_CHECK(env.get_entities().size() == 0);
    }

    SUBCASE("Bullets destroyed, stay destroyed")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        entity_states = {};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

        std::vector<std::unique_ptr<IEvent>> events;
        events.push_back(std::make_unique<EntityDestroyed>(0, 0.05, Transform{1, 2, 1}));
        playback_env.add_events(std::move(events));

        playback_env.update(0.025);
        DOCTEST_CHECK(env.get_entities().size() == 1);

        playback_env.update(0.025);
        DOCTEST_CHECK(env.get_entities().size() == 0);

        playback_env.update(1);
        DOCTEST_CHECK(env.get_entities().size() == 0);
    }

    SUBCASE("Bullets that disappear without being explicitly destroyed disappear")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                  {{0, 0}, b2Rot(0)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

        entity_states = {};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

        playback_env.update(0.025);
        DOCTEST_CHECK(env.get_entities().size() == 1);

        playback_env.update(1);
        DOCTEST_CHECK(env.get_entities().size() == 0);
    }

    SUBCASE("HPs are updated correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 1}, b2Rot(0)},
                                                  {{2, 3}, b2Rot(1)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        agent_transforms = {{{1, 2}, b2Rot(1)},
                            {{3, 4}, b2Rot(0.5)}};
        playback_env.add_new_state({agent_transforms, entity_states, {9, 5}, {0, 0}, 1});

        playback_env.update(0);
        DOCTEST_CHECK(env.get_bodies()[0]->get_hp() == 10);
        DOCTEST_CHECK(env.get_bodies()[1]->get_hp() == 10);

        playback_env.update(0.05);
        DOCTEST_CHECK(env.get_bodies()[0]->get_hp() == 10);
        DOCTEST_CHECK(env.get_bodies()[1]->get_hp() == 10);

        playback_env.update(0.06);
        DOCTEST_CHECK(env.get_bodies()[0]->get_hp() == 9);
        DOCTEST_CHECK(env.get_bodies()[1]->get_hp() == 5);
    }

    SUBCASE("Scores are updated correctly")
    {
        auto &env = playback_env.get_env();

        std::vector<b2Transform> agent_transforms{{{0, 1}, b2Rot(0)},
                                                  {{2, 3}, b2Rot(1)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

        agent_transforms = {{{1, 2}, b2Rot(1)},
                            {{3, 4}, b2Rot(0.5)}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {5, -5}, 1});

        playback_env.update(0);
        DOCTEST_CHECK(env.get_scores()[0] == 0);
        DOCTEST_CHECK(env.get_scores()[1] == 0);

        playback_env.update(0.05);
        DOCTEST_CHECK(env.get_scores()[0] == 0);
        DOCTEST_CHECK(env.get_scores()[1] == 0);

        playback_env.update(0.06);
        DOCTEST_CHECK(env.get_scores()[0] == 5);
        DOCTEST_CHECK(env.get_scores()[1] == -5);
    }

    SUBCASE("Timer counts down correctly")
    {
        std::vector<b2Transform> agent_transforms{{{0, 1}, b2Rot(0)},
                                                  {{2, 3}, b2Rot(1)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        for (int i = 0; i < 30; ++i)
        {
            playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, i});
        }

        playback_env.update(0);
        DOCTEST_CHECK(playback_env.get_render_data().texts[2].text == "10");

        playback_env.update(0.9);
        DOCTEST_CHECK(playback_env.get_render_data().texts[2].text == "10");

        playback_env.update(0.2);
        DOCTEST_CHECK(playback_env.get_render_data().texts[2].text == "9");

        playback_env.update(1);
        DOCTEST_CHECK(playback_env.get_render_data().texts[2].text == "8");
    }

    SUBCASE("Real-world example")
    {
        // 1
        std::vector<b2Transform> agent_transforms{{{0, -15}, b2Rot(0)},
                                                  {{0, 15}, b2Rot(-3.14159)}};
        std::unordered_map<unsigned int, b2Transform> entity_states{};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

        // 2
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 2});

        // 3
        agent_transforms = {{{0, -15}, b2Rot(0)},
                            {{1.39945e-08, 14.9565}, b2Rot(-3.14159)}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 3});

        // 4
        agent_transforms = {{{-1.04726e-08, -14.9565}, b2Rot(-1.95266e-09)},
                            {{2.7989e-08, 14.913}, b2Rot(-3.14159)}};
        entity_states = {{46869, {{1, -14.0895}, b2Rot(0)}},
                         {55328, {{1, 14.046}, b2Rot(0)}},
                         {38852, {{-1, -14.0895}, b2Rot(0)}}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 4});

        // 5
        agent_transforms = {{{-2.09451e-08, -14.913}, b2Rot(-1.95266e-09)},
                            {{4.19834e-08, 14.8696}, b2Rot(-3.14159)}};
        entity_states = {};
        std::vector<std::unique_ptr<IEvent>> events;
        events.push_back(std::make_unique<EntityDestroyed>(55328, 0.483334,
                                                           Transform{1, 10.395, 0}));
        events.push_back(std::make_unique<EntityDestroyed>(46869, 0.483334,
                                                           Transform{1, -9.995, 0}));
        events.push_back(std::make_unique<EntityDestroyed>(38852, 0.483334,
                                                           Transform{-1, -9.995, 0}));
        playback_env.add_events(std::move(events));
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 5});

        // 6
        agent_transforms = {{{-3.14177e-08, -14.8696}, b2Rot(-5.85799e-09)},
                            {{6.99724e-08, 14.7826}, b2Rot(-3.14159)}};
        playback_env.add_new_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 6});

        double time = 0;
        while (time < 0.6)
        {
            double delta_time = 1. / 60.;
            playback_env.update(delta_time);
            time += delta_time;
        }
    }
}
}