#include <iostream>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <spdlog/spdlog.h>
#include <torch/torch.h>

#include "training/environments/koth_env.h"
#include "graphics/colors.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"
#include "training/entities/ientity.h"
#include "training/entities/hill.h"
#include "training/entities/wall.h"
#include "training/rigid_body.h"
#include "training/training_program.h"
#include "misc/random.h"

namespace SingularityTrainer
{
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
      max_steps(max_steps),
      rng(std::move(rng)),
      world(std::move(world)),
      hill(std::make_unique<Hill>(0, 0, *this->world, *this)),
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
                    change_reward(body.first, reward_config.hill_tick_reward);
                    change_score(body.first, 1);
                }
                else
                {
                    change_reward(body.first, reward_config.enemy_hill_tick_punishment);
                }
            }
        }
    });
}

KothEnv::~KothEnv() {}

void KothEnv::add_entity(std::unique_ptr<IEntity> entity)
{
    entities.push_back(std::move(entity));
}

void KothEnv::change_score(Body *body, float score_delta)
{
    scores[body_numbers[body]] += score_delta;
}

float KothEnv::get_elapsed_time() const
{
    return elapsed_time;
}

RenderData KothEnv::get_render_data(bool lightweight)
{
    RenderData render_data;

    render_data.append(hill->get_render_data(lightweight));

    auto body_1_render_data = body_1->get_render_data(lightweight);
    render_data.append(body_1_render_data);
    auto body_2_render_data = body_2->get_render_data(lightweight);
    render_data.append(body_2_render_data);

    for (auto &wall : walls)
    {
        auto wall_render_data = wall->get_render_data(lightweight);
        render_data.append(wall_render_data);
    }

    for (auto &entity : entities)
    {
        render_data.append(entity->get_render_data());
    }

    return render_data;
}

StepInfo KothEnv::step(const std::vector<torch::Tensor> actions, float step_length)
{
    // Act
    auto actions_tensor_1 = actions[0].to(torch::kInt).contiguous();
    std::vector<int> actions_1(actions_tensor_1.data<int>(), actions_tensor_1.data<int>() + actions_tensor_1.numel());
    auto actions_tensor_2 = actions[1].to(torch::kInt).contiguous();
    std::vector<int> actions_2(actions_tensor_2.data<int>(), actions_tensor_2.data<int>() + actions_tensor_2.numel());
    body_1->act(actions_1);
    body_2->act(actions_2);

    // Update entities
    for (const auto &entity : entities)
    {
        entity->update();
    }

    // Destroy destroyable entities
    for (auto iter = entities.begin(); iter != entities.end();)
    {
        if ((*iter)->should_destroy())
        {
            (*iter)->destroy();
            iter = entities.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    // Step simulation
    world->Step(step_length, 3, 2);
    elapsed_time += step_length;

    // Hill score
    hill->update();

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
        {torch::from_blob(observation_1.data(), {static_cast<long>(observation_1.size())})
             .clone(),
         torch::from_blob(observation_2.data(), {static_cast<long>(observation_2.size())})
             .clone()},
        torch::from_blob(rewards.data(), {2, 1}, torch::kFloat).clone(),
        torch::from_blob(&done, {1, 1}, torch::kBool).to(torch::kFloat).expand({2, 1}),
        victor};

    // Reset reward
    rewards = {0, 0};

    // Return
    return step_info;
}

void KothEnv::forward(float step_length)
{
    world->Step(step_length, 3, 2);
    elapsed_time += step_length;
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

StepInfo KothEnv::reset()
{
    done = false;
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

    return StepInfo{{torch::from_blob(observation_1.data(), {static_cast<long>(observation_1.size())})
                         .clone(),
                     torch::from_blob(observation_2.data(), {static_cast<long>(observation_2.size())})
                         .clone()},
                    torch::from_blob(rewards.data(), {2, 1}, torch::kFloat).clone(),
                    torch::from_blob(&done, {1, 1}, torch::kBool).to(torch::kFloat).expand({2, 1})};
}
}
