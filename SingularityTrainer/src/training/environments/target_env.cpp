#include <iostream>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <spdlog/spdlog.h>

#include "training/environments/target_env.h"
#include "graphics/colors.h"
#include "training/agents/test_agent.h"
#include "training/entities/bullet.h"
#include "training/entities/target.h"
#include "training/entities/wall.h"
#include "training/rigid_body.h"

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
                case RigidBody::ParentTypes::Agent:
                    static_cast<IAgent *>(body->parent)->begin_contact(other);
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
                case RigidBody::ParentTypes::Agent:
                    static_cast<IAgent *>(body->parent)->end_contact(other);
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
                }
            }
        }
    }
};

TargetEnv::TargetEnv(ResourceManager &resource_manager, float x, float y, float scale, int max_steps, int seed)
    : max_steps(max_steps),
      command_queue_flag(0),
      reward(0),
      step_counter(0),
      done(false),
      rng(seed),
      elapsed_time(0)
{
    // Box2D world
    world = std::make_unique<b2World>(b2Vec2(0, 0));
    contact_listener = std::make_unique<ContactListener>();
    world->SetContactListener(contact_listener.get());

    // Agent
    agent = std::make_unique<TestAgent>(resource_manager, *world, &rng);

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
    ThreadCommand command{Commands::Stop};
    command_queue.emplace(std::move(command));
    command_queue_flag++;
    thread->join();
};

float TargetEnv::get_elapsed_time() const
{
    return elapsed_time;
}

void TargetEnv::start_thread()
{
    thread = new std::thread(&TargetEnv::thread_loop, this);
}

RenderData TargetEnv::get_render_data(bool lightweight)
{
    RenderData render_data;

    auto agent_render_data = agent->get_render_data(lightweight);
    render_data.append(agent_render_data);

    for (auto &wall : walls)
    {
        auto wall_render_data = wall->get_render_data(lightweight);
        render_data.append(wall_render_data);
    }

    auto target_render_data = target->get_render_data(lightweight);
    render_data.append(target_render_data);

    return render_data;
}

std::future<std::unique_ptr<StepInfo>> TargetEnv::step(std::vector<int> &actions, float step_length)
{
    std::promise<std::unique_ptr<StepInfo>> promise;
    std::future<std::unique_ptr<StepInfo>> future = promise.get_future();
    ThreadCommand command{Commands::Step, std::move(promise), step_length, actions};
    command_queue.emplace(std::move(command));
    command_queue_flag++;

    return future;
}

void TargetEnv::forward(float step_length)
{
    std::promise<std::unique_ptr<StepInfo>> promise;
    ThreadCommand command{Commands::Forward, std::move(promise), step_length};
    command_queue.emplace(std::move(command));
    command_queue_flag++;
}

void TargetEnv::change_reward(float reward_delta)
{
    reward += reward_delta;
    total_reward += reward_delta;
}

void TargetEnv::set_done()
{
    done = true;
}

std::future<std::unique_ptr<StepInfo>> TargetEnv::reset()
{
    std::promise<std::unique_ptr<StepInfo>> promise;
    std::future<std::unique_ptr<StepInfo>> future = promise.get_future();
    ThreadCommand command{Commands::Reset, std::move(promise)};
    command_queue.emplace(std::move(command));
    command_queue_flag++;
    return future;
}

void TargetEnv::thread_loop()
{
    bool quit = false;
    while (!quit)
    {
        while (command_queue_flag == 0)
        {
        }
        ThreadCommand command = std::move(command_queue.front());
        command_queue.pop();
        command_queue_flag--;
        std::unique_ptr<StepInfo> step_info = std::make_unique<StepInfo>();
        switch (command.command)
        {
        case Commands::Stop:
            quit = true;
            break;
        case Commands::Step:
            // Act
            agent->act(command.actions);

            // Step simulation
            world->Step(command.step_length, 3, 2);
            elapsed_time += command.step_length;

            // Max episode length
            step_counter++;
            done = done || step_counter >= max_steps;

            // Return step information
            step_info->observation = agent->get_observation();
            step_info->reward = reward;
            step_info->done = done;

            // Reset reward
            reward = 0;

            // Increment step counter
            if (done)
            {
                spdlog::info("Reward: {}", total_reward);
                reset();
            }

            // Return
            command.promise.set_value(std::move(step_info));
            break;
        case Commands::Forward:
            world->Step(command.step_length, 3, 2);
            elapsed_time += command.step_length;
            break;
        case Commands::Reset:
            done = false;
            reward = 0;
            total_reward = 0;
            step_counter = 0;

            // Reset agent position
            agent->rigid_body->body->SetTransform(b2Vec2_zero, 0);
            agent->rigid_body->body->SetAngularVelocity(0);
            agent->rigid_body->body->SetLinearVelocity(b2Vec2_zero);

            // Change target position
            float target_x = rng.next_float(0, 18) - 9;
            float target_y = rng.next_float(0, 18) - 9;
            target->rigid_body->body->SetTransform(b2Vec2(target_x, target_y), 0);

            // Fill in StepInfo
            step_info->observation = agent->get_observation();
            step_info->done = done;
            step_info->reward = reward;

            // Return
            command.promise.set_value(std::move(step_info));
            break;
        }
    }
}
}