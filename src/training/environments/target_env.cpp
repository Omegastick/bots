#include <iostream>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>

#include "training/environments/target_env.h"
#include "graphics/colors.h"
#include "training/bodies/test_body.h"
#include "training/entities/bullet.h"
#include "training/entities/target.h"
#include "training/entities/wall.h"
#include "training/rigid_body.h"
#include "misc/random.h"

namespace SingularityTrainer
{
class ContactListener : public b2ContactListener
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
                RigidBody *other = bodies[1 - i];

                switch (body->parent_type)
                {
                case RigidBody::ParentTypes::Body:
                    static_cast<Body *>(body->parent)->begin_contact(other);
                    break;
                case RigidBody::ParentTypes::Bullet:
                    static_cast<Bullet *>(body->parent)->begin_contact(other);
                    break;
                case RigidBody::ParentTypes::Target:
                    static_cast<Target *>(body->parent)->begin_contact(other);
                    break;
                case RigidBody::ParentTypes::Wall:
                    static_cast<Wall *>(body->parent)->begin_contact(other);
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
                RigidBody *other = bodies[1 - i];

                switch (body->parent_type)
                {
                case RigidBody::ParentTypes::Body:
                    static_cast<Body *>(body->parent)->end_contact(other);
                    break;
                case RigidBody::ParentTypes::Bullet:
                    static_cast<Bullet *>(body->parent)->end_contact(other);
                    break;
                case RigidBody::ParentTypes::Target:
                    static_cast<Target *>(body->parent)->end_contact(other);
                    break;
                case RigidBody::ParentTypes::Wall:
                    static_cast<Wall *>(body->parent)->end_contact(other);
                    break;
                default:
                    break;
                }
            }
        }
    }
};

TargetEnv::TargetEnv(float /*x*/, float /*y*/, float /*scale*/, int max_steps, int seed)
    : max_steps(max_steps),
      reward(0),
      step_counter(0),
      done(false),
      rng(std::make_unique<Random>(seed)),
      elapsed_time(0)
{
    // Box2D world
    world = std::make_unique<b2World>(b2Vec2(0, 0));
    contact_listener = std::make_unique<ContactListener>();
    world->SetContactListener(contact_listener.get());

    // Body
    body = std::make_unique<TestBody>(*world, *rng, *this);

    // Target
    target = std::make_unique<Target>(4, 4, *world, *this);

    // Walls
    walls.push_back(std::make_unique<Wall>(-10, -10, 20, 0.1, *world));
    walls.push_back(std::make_unique<Wall>(-10, -10, 0.1, 20, *world));
    walls.push_back(std::make_unique<Wall>(-10, 9.9, 20, 0.1, *world));
    walls.push_back(std::make_unique<Wall>(9.9, -10, 0.1, 20, *world));
}

TargetEnv::~TargetEnv()
{
    ThreadCommand command{Commands::Stop, {}, {}, {}};
    {
        std::lock_guard lock_guard(command_queue_mutex);
        command_queue.push(std::move(command));
        command_queue_condvar.notify_one();
    }
    if (thread != nullptr)
    {
        thread->join();
    }
}

float TargetEnv::get_elapsed_time() const
{
    return elapsed_time;
}

void TargetEnv::start_thread()
{
    thread = new std::thread(&TargetEnv::thread_loop, this);
}

void TargetEnv::draw(Renderer &renderer, bool lightweight)
{
    body->draw(renderer, lightweight);

    for (auto &wall : walls)
    {
        wall->draw(renderer, lightweight);
    }

    target->draw(renderer, lightweight);
}

std::future<StepInfo> TargetEnv::step(torch::Tensor actions, float step_length)
{
    std::promise<StepInfo> promise;
    std::future<StepInfo> future = promise.get_future();
    ThreadCommand command{Commands::Step, std::move(promise), step_length, actions};
    {
        std::lock_guard lock_guard(command_queue_mutex);
        command_queue.push(std::move(command));
        command_queue_condvar.notify_one();
    }
    return future;
}

void TargetEnv::forward(float step_length)
{
    std::promise<StepInfo> promise;
    ThreadCommand command{Commands::Forward, std::move(promise), step_length, {}};
    {
        std::lock_guard lock_guard(command_queue_mutex);
        command_queue.push(std::move(command));
        command_queue_condvar.notify_one();
    }
}

void TargetEnv::change_reward(int /*body*/, float reward_delta)
{
    reward += reward_delta;
    total_reward += reward_delta;
}

void TargetEnv::change_reward(Body * /*body*/, float reward_delta)
{
    change_reward(0, reward_delta);
}

void TargetEnv::set_done()
{
    done = true;
}

std::future<StepInfo> TargetEnv::reset()
{
    std::promise<StepInfo> promise;
    std::future<StepInfo> future = promise.get_future();
    ThreadCommand command{Commands::Reset, std::move(promise), {}, {}};
    {
        std::lock_guard lock_guard(command_queue_mutex);
        command_queue.push(std::move(command));
        command_queue_condvar.notify_one();
    }
    return future;
}

void TargetEnv::thread_loop()
{
    bool quit = false;
    while (!quit)
    {
        std::unique_lock lock(command_queue_mutex);
        command_queue_condvar.wait(lock, [this] { return !command_queue.empty(); });
        auto command = std::move(command_queue.front());
        command_queue.pop();
        if (command.command == Commands::Stop)
        {
            quit = true;
        }
        else if (command.command == Commands::Step)
        {
            // Act
            auto actions_tensor = command.actions[0].to(torch::kInt).contiguous();
            std::vector<int> actions(actions_tensor.data<int>(), actions_tensor.data<int>() + actions_tensor.numel());
            body->act(actions);

            // Step simulation
            world->Step(command.step_length, 3, 2);
            elapsed_time += command.step_length;

            // Max episode length
            step_counter++;
            done = done || step_counter >= max_steps;

            // Return step information
            auto observation = body->get_observation();
            StepInfo step_info{
                torch::from_blob(observation.data(), {1, static_cast<long>(observation.size())}).clone(),
                torch::from_blob(&reward, {1, 1}, torch::kFloat).clone(),
                torch::from_blob(&done, {1, 1}, torch::kBool).to(torch::kFloat)};

            // Reset reward
            reward = 0;

            // Increment step counter
            if (done)
            {
                reset_impl();
            }

            // Return
            command.promise.set_value(step_info);
        }
        else if (command.command == Commands::Forward)
        {
            world->Step(command.step_length, 3, 2);
            elapsed_time += command.step_length;
        }
        else if (command.command == Commands::Reset)
        {
            reset_impl();

            // Fill in StepInfo
            auto observation = body->get_observation();
            StepInfo step_info{
                torch::from_blob(observation.data(), {1, static_cast<long>(observation.size())}).clone(),
                torch::from_blob(&reward, {1, 1}, torch::kFloat).clone(),
                torch::from_blob(&done, {1, 1}, torch::kBool).to(torch::kFloat)};

            // Return
            command.promise.set_value(std::move(step_info));
        }
    }
}

void TargetEnv::reset_impl()
{
    done = false;
    reward = 0;
    total_reward = 0;
    step_counter = 0;

    // Reset body position
    body->get_rigid_body().body->SetTransform(b2Vec2_zero, 0);
    body->get_rigid_body().body->SetAngularVelocity(0);
    body->get_rigid_body().body->SetLinearVelocity(b2Vec2_zero);

    // Change target position
    float target_x = rng->next_float(0, 18) - 9;
    float target_y = rng->next_float(0, 18) - 9;
    target->rigid_body->body->SetTransform(b2Vec2(target_x, target_y), 0);
}
}