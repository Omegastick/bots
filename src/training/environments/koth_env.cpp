#include <iostream>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <spdlog/spdlog.h>
#include <torch/torch.h>

#include "koth_env.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "training/bodies/test_body.h"
#include "training/effects/ieffect.h"
#include "training/entities/bullet.h"
#include "training/entities/ientity.h"
#include "training/entities/hill.h"
#include "training/entities/wall.h"
#include "training/events/ievent.h"
#include "training/rigid_body.h"
#include "training/training_program.h"
#include "misc/random.h"

namespace SingularityTrainer
{
constexpr std::size_t max_effects = 10000;

class KothContactListener : public b2ContactListener
{
    void BeginContact(b2Contact *contact)
    {
        // Only collide if both objects are registered colliders
        void *user_data_a = contact->GetFixtureA()->GetBody()->GetUserData();
        void *user_data_b = contact->GetFixtureB()->GetBody()->GetUserData();
        if (user_data_a && user_data_b)
        {
            // Cast the objects to RigidBodies
            RigidBody *bodies[2]{static_cast<RigidBody *>(user_data_a), static_cast<RigidBody *>(user_data_b)};

            // Perform the same operation for both the bodies, with the opposite body being the "other"
            for (int i = 0; i < 2; i++)
            {
                RigidBody *body = bodies[i];
                RigidBody *other = bodies[(i + 1) % 2];

                switch (body->parent_type)
                {
                case RigidBody::ParentTypes::Body:
                    static_cast<Body *>(body->parent)->begin_contact(other);
                    break;
                case RigidBody::ParentTypes::Bullet:
                    static_cast<Bullet *>(body->parent)->begin_contact(other);
                    break;
                case RigidBody::ParentTypes::Wall:
                    static_cast<Wall *>(body->parent)->begin_contact(other);
                    break;
                case RigidBody::ParentTypes::Hill:
                    static_cast<Hill *>(body->parent)->begin_contact(other);
                    break;
                default:
                    break;
                }
            }
        }
    }

    void EndContact(b2Contact *contact)
    {
        void *user_data_a = contact->GetFixtureA()->GetBody()->GetUserData();
        void *user_data_b = contact->GetFixtureB()->GetBody()->GetUserData();

        if (user_data_a && user_data_b)
        {
            RigidBody *bodies[2]{static_cast<RigidBody *>(user_data_a), static_cast<RigidBody *>(user_data_b)};
            for (int i = 0; i < 2; i++)
            {
                RigidBody *body = bodies[i];
                RigidBody *other = bodies[(i + 1) % 2];

                switch (body->parent_type)
                {
                case RigidBody::ParentTypes::Body:
                    static_cast<Body *>(body->parent)->end_contact(other);
                    break;
                case RigidBody::ParentTypes::Bullet:
                    static_cast<Bullet *>(body->parent)->end_contact(other);
                    break;
                case RigidBody::ParentTypes::Wall:
                    static_cast<Wall *>(body->parent)->end_contact(other);
                    break;
                case RigidBody::ParentTypes::Hill:
                    static_cast<Hill *>(body->parent)->end_contact(other);
                    break;
                default:
                    break;
                }
            }
        }
    }
};

KothEnv::KothEnv(int max_steps,
                 std::unique_ptr<Body> body_1,
                 std::unique_ptr<Body> body_2,
                 std::unique_ptr<b2World> world,
                 std::unique_ptr<Random> rng,
                 RewardConfig reward_config)
    : body_1(std::move(body_1)),
      body_2(std::move(body_2)),
      effects(),
      entities(),
      max_steps(max_steps),
      rng(std::move(rng)),
      world(std::move(world)),
      hill(std::make_unique<Hill>(0, 0, *this->world)),
      elapsed_time(0),
      done(false),
      rewards({0, 0}),
      scores({0, 0}),
      step_counter(0),
      reward_config(reward_config)
{
    // Box2D world
    contact_listener = std::make_unique<KothContactListener>();
    this->world->SetContactListener(contact_listener.get());

    // Bodys
    this->body_1->set_environment(*this);
    this->body_2->set_environment(*this);
    this->body_1->set_rng(*this->rng);
    this->body_2->set_rng(*this->rng);
    body_numbers[this->body_1.get()] = 0;
    body_numbers[this->body_2.get()] = 1;

    // Walls
    walls.push_back(std::make_unique<Wall>(-10, -20, 20, 0.1, *this->world));
    walls.push_back(std::make_unique<Wall>(-10, -20, 0.1, 40, *this->world));
    walls.push_back(std::make_unique<Wall>(-10, 19.9, 20, 0.1, *this->world));
    walls.push_back(std::make_unique<Wall>(9.9, -20, 0.1, 40, *this->world));
    walls.push_back(std::make_unique<Wall>(-2.5, -9.9, 5, 0.2, *this->world));
    walls.push_back(std::make_unique<Wall>(-2.5, 10.1, 5, 0.2, *this->world));

    hill->register_callback([&](const std::unordered_map<Body *, int> &bodies) {
        if (bodies.size() == 1)
        {
            for (auto body : bodies)
            {
                if (body.second > 0)
                {
                    spdlog::debug("Rewarding agent {}", reinterpret_cast<intptr_t>(body.first));
                    int body_number = body_numbers[body.first];
                    change_reward(body_number, this->reward_config.hill_tick_reward);
                    change_score(body_number, 1);

                    int opponent_body_number = (body_number + 1) % 2;
                    change_reward(opponent_body_number,
                                  this->reward_config.enemy_hill_tick_punishment);
                }
            }
        }
        else if (const auto body_count = bodies.size(); body_count > 1)
        {
            spdlog::debug("{} agents on the hill", body_count);
        }
    });
}

void KothEnv::add_effect(std::unique_ptr<IEffect> effect)
{
    if (effects.size() > max_effects) {
        return;
    }
    effects.push_back(std::move(effect));
}

void KothEnv::add_entity(std::unique_ptr<IEntity> entity)
{
    entities[entity->get_id()] = std::move(entity);
}

void KothEnv::add_event(std::unique_ptr<IEvent> event)
{
    events.push_back(std::move(event));
}

void KothEnv::change_score(Body *body, float score_delta)
{
    change_score(body_numbers[body], score_delta);
}

void KothEnv::change_score(int body, float score_delta)
{
    scores[body] += score_delta;
}

void KothEnv::clear_effects()
{
    effects.clear();
}

double KothEnv::get_elapsed_time() const
{
    return elapsed_time;
}

void KothEnv::draw(Renderer &renderer, bool lightweight)
{
    hill->draw(renderer, lightweight);

    body_1->draw(renderer, lightweight);
    body_2->draw(renderer, lightweight);

    for (auto &wall : walls)
    {
        wall->draw(renderer, lightweight);
    }

    for (auto &entity : entities)
    {
        entity.second->draw(renderer);
    }

    if (!lightweight)
    {
        for (auto &effect : effects)
        {
            effect->trigger(renderer);
        }
    }
    effects.clear();

    // Health bars
    for (const auto &body : get_bodies())
    {
        if (body->get_modules().empty())
        {
            continue;
        }
        const auto hp = static_cast<double>(body->get_hp());
        const auto hp_ratio = hp / 10.;

        const auto b2_position = body->get_rigid_body().body->GetPosition();
        const glm::vec2 position(b2_position.x, b2_position.y);
        const double full_bar_width = 3;
        const auto bar_width = full_bar_width * hp_ratio;
        Line health_bar{{{position + glm::vec2{-full_bar_width * 0.5, -2}},
                         {position + glm::vec2{full_bar_width * 0.5, -2}}},
                        {{0.1f, 0.1f}},
                        {{set_alpha(cl_base0, 0.5), set_alpha(cl_base0, 0.5)}}};
        renderer.draw(health_bar);
        health_bar.points = {{position + glm::vec2{-bar_width * 0.5, -2}},
                             {position + glm::vec2{bar_width * 0.5, -2}}};
        health_bar.colors = {cl_red, cl_red};
        renderer.draw(health_bar);
    }

    // Timer
    double max_time = max_steps / 10.f;
    int seconds_remaining = static_cast<int>(std::ceil(max_time - elapsed_time));
    Text timer;
    timer.text = std::to_string(seconds_remaining);
    timer.font = "roboto-32";
    timer.color = cl_white;
    timer.transform.set_position({0, 0});
    timer.transform.set_scale({0.0725, 0.0725});
    const double character_width = 1.1;
    double width = character_width * timer.text.size();
    const double height = 1.2;
    timer.transform.set_origin({width / 2., height / 2.});
    renderer.draw(timer);
}

StepInfo KothEnv::step(const std::vector<torch::Tensor> actions, float step_length)
{
    // Act
    auto actions_tensor_1 = actions[0].to(torch::kInt).contiguous();
    std::vector<int> actions_1(actions_tensor_1.data_ptr<int>(), actions_tensor_1.data_ptr<int>() + actions_tensor_1.numel());
    auto actions_tensor_2 = actions[1].to(torch::kInt).contiguous();
    std::vector<int> actions_2(actions_tensor_2.data_ptr<int>(), actions_tensor_2.data_ptr<int>() + actions_tensor_2.numel());
    body_1->act(actions_1);
    body_2->act(actions_2);

    // Update entities
    for (const auto &entity : entities)
    {
        entity.second->update();
    }

    // Hill score
    hill->update();

    // Step simulation
    forward(step_length);

    // Check if body is destroyed
    if (body_1->get_hp() <= 0)
    {
        change_reward(body_1.get(), reward_config.loss_punishment);
        change_score(body_2.get(), 100);
        change_reward(body_2.get(), reward_config.victory_reward);
        set_done();
    }
    else if (body_2->get_hp() <= 0)
    {
        change_reward(body_1.get(), reward_config.victory_reward);
        change_score(body_1.get(), 100);
        set_done();
    }

    // Max episode length
    step_counter++;
    done = done || step_counter >= max_steps;
    int victor = -1;
    if (done)
    {
        if (scores[0] > scores[1])
        {
            change_reward(body_1.get(), reward_config.victory_reward);
            change_reward(body_2.get(), reward_config.loss_punishment);
            victor = 0;
        }
        else if (scores[1] > scores[0])
        {
            change_reward(body_1.get(), reward_config.loss_punishment);
            change_reward(body_2.get(), reward_config.victory_reward);
            victor = 1;
        }
    }

    // Return step information
    auto observation_1 = body_1->get_observation();
    auto observation_2 = body_2->get_observation();
    StepInfo step_info{
        std::move(events),
        {torch::from_blob(observation_1.data(), {static_cast<long>(observation_1.size())})
             .clone(),
         torch::from_blob(observation_2.data(), {static_cast<long>(observation_2.size())})
             .clone()},
        torch::from_blob(rewards.data(), {2, 1}, torch::kFloat).clone(),
        torch::from_blob(&done, {1, 1}, torch::kBool).to(torch::kFloat).expand({2, 1}),
        victor};
    events.clear();

    // Reset reward
    rewards = {0, 0};

    // Return
    return step_info;
}

void KothEnv::forward(float step_length)
{
    world->Step(step_length, 3, 2);
    double step_start_time = elapsed_time;
    elapsed_time += step_length;
    body_1->sub_update();
    body_2->sub_update();

    for (const auto &event : events)
    {
        double scheduled_time = event->get_time();
        if (scheduled_time >= step_start_time && scheduled_time < elapsed_time)
        {
            event->trigger(*this);
        }
    }
}

void KothEnv::change_reward(int body, float reward_delta)
{
    rewards[body] += reward_delta;
}

void KothEnv::change_reward(Body *body, float reward_delta)
{
    change_reward(body_numbers[body], reward_delta);
}

void KothEnv::set_done()
{
    done = true;
}

void KothEnv::set_state(const EnvState &state)
{
    // Move bodies
    for (unsigned int i = 0; i < state.agent_transforms.size(); ++i)
    {
        auto &body = *get_bodies()[i]->get_rigid_body().body;
        body.SetTransform(state.agent_transforms[i].p,
                          state.agent_transforms[i].q.GetAngle());
    }

    // Remove old bullets
    for (auto it = entities.cbegin(), next_it = it; it != entities.cend(); it = next_it)
    {
        ++next_it;
        if (state.entity_states.find(it->first) == state.entity_states.end())
        {
            it->second->destroy();
        }
    }

    // Add/Update bullet positions
    for (const auto bullet : state.entity_states)
    {
        auto found_entity = entities.find(bullet.first);
        if (found_entity != entities.end())
        {
            // Update position
            found_entity->second->set_transform(bullet.second.p,
                                                bullet.second.q.GetAngle());
        }
        else
        {
            // Make new bullet
            add_entity(std::make_unique<Bullet>(b2Vec2{0, 0},
                                                b2Vec2{0, 0},
                                                *world,
                                                body_1.get(),
                                                bullet.first,
                                                *this));
            entities[bullet.first]->set_transform(bullet.second.p,
                                                  bullet.second.q.GetAngle());
        }
    }

    // Set HPs
    for (unsigned int i = 0; i < state.hps.size(); ++i)
    {
        get_bodies()[i]->set_hp(state.hps[i]);
    }

    // Set scores
    scores = state.scores;
}

StepInfo KothEnv::reset()
{
    done = false;
    elapsed_time = 0;
    rewards = {0, 0};
    scores = {0, 0};
    step_counter = 0;

    // Reset body position
    body_1->get_rigid_body().body->SetTransform({0, -15}, 0);
    body_1->get_rigid_body().body->SetAngularVelocity(0);
    body_1->get_rigid_body().body->SetLinearVelocity(b2Vec2_zero);
    body_2->get_rigid_body().body->SetTransform({0, 15}, glm::radians(180.f));
    body_2->get_rigid_body().body->SetAngularVelocity(0);
    body_2->get_rigid_body().body->SetLinearVelocity(b2Vec2_zero);

    // Reset body hp
    body_1->reset();
    body_2->reset();

    // Reset hill
    hill->reset();

    auto observation_1 = body_1->get_observation();
    auto observation_2 = body_2->get_observation();

    StepInfo step_info{std::move(events),
                       {torch::from_blob(observation_1.data(), {static_cast<long>(observation_1.size())})
                            .clone(),
                        torch::from_blob(observation_2.data(), {static_cast<long>(observation_2.size())})
                            .clone()},
                       torch::from_blob(rewards.data(), {2, 1}, torch::kFloat).clone(),
                       torch::from_blob(&done, {1, 1}, torch::kBool).to(torch::kFloat).expand({2, 1})};
    events.clear();

    return step_info;
}

TEST_CASE("KothEnv")
{
    Random rng(0);
    TestBodyFactory body_factory(rng);
    KothEnvFactory env_factory(100, body_factory);
    auto env = env_factory.make();

    SUBCASE("set_state()")
    {
        SUBCASE("New bullets are added correctly")
        {
            std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                      {{0, 0}, b2Rot(0)}};
            std::unordered_map<unsigned int, b2Transform> entity_states{};
            env->set_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});
            DOCTEST_CHECK(env->get_entities().size() == 0);

            entity_states = {{0, {{0, 0}, b2Rot(0)}},
                             {1, {{1, 1}, b2Rot(1)}}};
            env->set_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});
            DOCTEST_CHECK(env->get_entities().size() == 2);
        }

        SUBCASE("Bullets are moved correctly")
        {
            std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                      {{0, 0}, b2Rot(0)}};
            std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}},
                                                                        {1, {{1, 1}, b2Rot(1)}}};
            env->set_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

            auto bullet_0_tranform = env->get_entities()[0]->get_transform();
            DOCTEST_CHECK(bullet_0_tranform.p.x == doctest::Approx(0));
            DOCTEST_CHECK(bullet_0_tranform.p.y == doctest::Approx(0));
            DOCTEST_CHECK(bullet_0_tranform.q.GetAngle() == doctest::Approx(0));
            auto bullet_1_tranform = env->get_entities()[1]->get_transform();
            DOCTEST_CHECK(bullet_1_tranform.p.x == doctest::Approx(1));
            DOCTEST_CHECK(bullet_1_tranform.p.y == doctest::Approx(1));
            DOCTEST_CHECK(bullet_1_tranform.q.GetAngle() == doctest::Approx(1));

            entity_states = {{0, {{0.5, 0.5}, b2Rot(0.5)}},
                             {1, {{-1, 0}, b2Rot(2)}}};
            env->set_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

            bullet_0_tranform = env->get_entities()[0]->get_transform();
            DOCTEST_CHECK(bullet_0_tranform.p.x == doctest::Approx(0.5));
            DOCTEST_CHECK(bullet_0_tranform.p.y == doctest::Approx(0.5));
            DOCTEST_CHECK(bullet_0_tranform.q.GetAngle() == doctest::Approx(0.5));
            bullet_1_tranform = env->get_entities()[1]->get_transform();
            DOCTEST_CHECK(bullet_1_tranform.p.x == doctest::Approx(-1));
            DOCTEST_CHECK(bullet_1_tranform.p.y == doctest::Approx(0));
            DOCTEST_CHECK(bullet_1_tranform.q.GetAngle() == doctest::Approx(2));
        }

        SUBCASE("Bullets are removed correctly")
        {
            std::vector<b2Transform> agent_transforms{{{0, 0}, b2Rot(0)},
                                                      {{0, 0}, b2Rot(0)}};
            std::unordered_map<unsigned int, b2Transform> entity_states{{0, {{0, 0}, b2Rot(0)}},
                                                                        {1, {{1, 1}, b2Rot(1)}}};
            env->set_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});
            DOCTEST_CHECK(env->get_entities().size() == 2);

            entity_states = {};
            env->set_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});
            DOCTEST_CHECK(env->get_entities().size() == 0);
        }

        SUBCASE("Bodies are moved correctly")
        {
            std::vector<b2Transform> agent_transforms{{{0, 1}, b2Rot(0)},
                                                      {{2, 3}, b2Rot(1)}};
            std::unordered_map<unsigned int, b2Transform> entity_states{};
            env->set_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 0});

            auto agent_0_tranform = env->get_bodies()[0]->get_rigid_body().body->GetTransform();
            DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(0));
            DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(1));
            DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(0));
            auto agent_1_tranform = env->get_bodies()[1]->get_rigid_body().body->GetTransform();
            DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(2));
            DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(3));
            DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(1));

            agent_transforms = {{{1, 2}, b2Rot(1)},
                                {{3, 4}, b2Rot(0.5)}};
            env->set_state({agent_transforms, entity_states, {10, 10}, {0, 0}, 1});

            agent_0_tranform = env->get_bodies()[0]->get_rigid_body().body->GetTransform();
            DOCTEST_CHECK(agent_0_tranform.p.x == doctest::Approx(1));
            DOCTEST_CHECK(agent_0_tranform.p.y == doctest::Approx(2));
            DOCTEST_CHECK(agent_0_tranform.q.GetAngle() == doctest::Approx(1));
            agent_1_tranform = env->get_bodies()[1]->get_rigid_body().body->GetTransform();
            DOCTEST_CHECK(agent_1_tranform.p.x == doctest::Approx(3));
            DOCTEST_CHECK(agent_1_tranform.p.y == doctest::Approx(4));
            DOCTEST_CHECK(agent_1_tranform.q.GetAngle() == doctest::Approx(0.5));
        }
    }
}
}
