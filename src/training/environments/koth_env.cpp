#include <iostream>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <spdlog/spdlog.h>
#include <torch/torch.h>

#include "training/environments/koth_env.h"
#include "graphics/colors.h"
#include "training/agents/test_agent.h"
#include "training/entities/bullet.h"
#include "training/entities/wall.h"
#include "training/entities/hill.h"
#include "training/rigid_body.h"
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
                RigidBody *other = bodies[1 - i];

                switch (body->parent_type)
                {
                case RigidBody::ParentTypes::Agent:
                    static_cast<Agent *>(body->parent)->begin_contact(other);
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
                RigidBody *other = bodies[1 - i];

                switch (body->parent_type)
                {
                case RigidBody::ParentTypes::Agent:
                    static_cast<Agent *>(body->parent)->end_contact(other);
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

KothEnv::KothEnv(int max_steps, Random &rng)
    : max_steps(max_steps),
      rng(std::move(rng)),
      world({0, 0}),
      hill(std::make_unique<Hill>(0, 0, world, *this)),
      elapsed_time(0),
      done(false),
      rewards({0, 0}),
      step_counter(0),
      total_rewards({0, 0})
{
    // Box2D world
    contact_listener = std::make_unique<KothContactListener>();
    world.SetContactListener(contact_listener.get());

    // Agent
    agent_1 = std::make_unique<TestAgent>(world, &rng, *this);
    agent_numbers[agent_1.get()] = 0;
    agent_2 = std::make_unique<TestAgent>(world, &rng, *this);
    agent_numbers[agent_2.get()] = 1;

    // Walls
    walls.push_back(std::make_unique<Wall>(-10, -20, 20, 0.1, world));
    walls.push_back(std::make_unique<Wall>(-10, -20, 0.1, 40, world));
    walls.push_back(std::make_unique<Wall>(-10, 19.9, 20, 0.1, world));
    walls.push_back(std::make_unique<Wall>(9.9, -20, 0.1, 40, world));
}

KothEnv::~KothEnv()
{
    ThreadCommand command{Commands::Stop, {}, {}, {}};
    {
        std::lock_guard lock_guard(command_queue_mutex);
        command_queue.push(std::move(command));
        command_queue_condvar.notify_one();
    }
    thread->join();
}

float KothEnv::get_elapsed_time() const
{
    return elapsed_time;
}

void KothEnv::start_thread()
{
    thread = std::make_unique<std::thread>(&KothEnv::thread_loop, this);
}

RenderData KothEnv::get_render_data(bool lightweight)
{
    RenderData render_data;

    render_data.append(hill->get_render_data(lightweight));

    auto agent_1_render_data = agent_1->get_render_data(lightweight);
    render_data.append(agent_1_render_data);
    auto agent_2_render_data = agent_2->get_render_data(lightweight);
    render_data.append(agent_2_render_data);

    for (auto &wall : walls)
    {
        auto wall_render_data = wall->get_render_data(lightweight);
        render_data.append(wall_render_data);
    }

    return render_data;
}

std::future<StepInfo> KothEnv::step(const torch::Tensor actions, float step_length)
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

void KothEnv::forward(float step_length)
{
    std::promise<StepInfo> promise;
    ThreadCommand command{Commands::Forward, std::move(promise), step_length, {}};
    {
        std::lock_guard lock_guard(command_queue_mutex);
        command_queue.push(std::move(command));
        command_queue_condvar.notify_one();
    }
}

void KothEnv::change_reward(int agent, float reward_delta)
{
    rewards[agent] += reward_delta;
    total_rewards[agent] += reward_delta;
}

void KothEnv::change_reward(Agent *agent, float reward_delta)
{
    change_reward(agent_numbers[agent], reward_delta);
}

void KothEnv::set_done()
{
    done = true;
}

std::future<StepInfo> KothEnv::reset()
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

void KothEnv::thread_loop()
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
            auto actions_tensor_1 = command.actions[0].to(torch::kInt).contiguous();
            std::vector<int> actions_1(actions_tensor_1.data<int>(), actions_tensor_1.data<int>() + actions_tensor_1.numel());
            auto actions_tensor_2 = command.actions[1].to(torch::kInt).contiguous();
            std::vector<int> actions_2(actions_tensor_2.data<int>(), actions_tensor_2.data<int>() + actions_tensor_2.numel());
            agent_1->act(actions_1);
            agent_2->act(actions_2);

            // Step simulation
            world.Step(command.step_length, 3, 2);
            elapsed_time += command.step_length;

            // Hill score
            hill->update();

            // Check if agent is destroyed
            if (agent_1->get_hp() < 0)
            {
                change_reward(agent_1.get(), -100);
                change_reward(agent_2.get(), 100);
                set_done();
            }
            else if (agent_2->get_hp() < 0)
            {
                change_reward(agent_1.get(), 100);
                change_reward(agent_2.get(), -100);
                set_done();
            }

            // Max episode length
            step_counter++;
            done = done || step_counter >= max_steps;

            // Return step information
            auto observation_1 = agent_1->get_observation();
            auto observation_2 = agent_2->get_observation();
            observation_1.insert(observation_1.begin(), observation_2.begin(),
                                 observation_2.end());
            StepInfo step_info{
                torch::from_blob(observation_1.data(),
                                 {2, static_cast<long>(observation_1.size()) / 2})
                    .clone(),
                torch::from_blob(rewards.data(), {2, 1}, torch::kFloat).clone(),
                torch::from_blob(&done, {1, 1}, torch::kBool).to(torch::kFloat).expand({2, 1})};

            // Reset reward
            rewards = {0, 0};

            // Increment step counter
            if (done)
            {
                reset_impl();
            }

            // Return
            command.promise.set_value(std::move(step_info));
        }
        else if (command.command == Commands::Forward)
        {
            world.Step(command.step_length, 3, 2);
            elapsed_time += command.step_length;
        }
        else if (command.command == Commands::Reset)
        {
            reset_impl();

            // Fill in StepInfo
            auto observation_1 = agent_1->get_observation();
            auto observation_2 = agent_2->get_observation();
            observation_1.insert(observation_1.begin(), observation_2.begin(),
                                 observation_2.end());
            StepInfo step_info{
                torch::from_blob(observation_1.data(),
                                 {2, static_cast<long>(observation_1.size()) / 2})
                    .clone(),
                torch::from_blob(rewards.data(), {2, 1}, torch::kFloat).clone(),
                torch::from_blob(&done, {1, 1}, torch::kBool).to(torch::kFloat).expand({2, 1})};

            // Return
            command.promise.set_value(std::move(step_info));
        }
    }
}

void KothEnv::reset_impl()
{
    done = false;
    rewards = {0, 0};
    total_rewards = {0, 0};
    step_counter = 0;

    // Reset agent position
    agent_1->get_rigid_body()->body->SetTransform({0, -10}, 0);
    agent_1->get_rigid_body()->body->SetAngularVelocity(0);
    agent_1->get_rigid_body()->body->SetLinearVelocity(b2Vec2_zero);
    agent_2->get_rigid_body()->body->SetTransform({0, 10}, glm::radians(180.f));
    agent_2->get_rigid_body()->body->SetAngularVelocity(0);
    agent_2->get_rigid_body()->body->SetLinearVelocity(b2Vec2_zero);

    // Reset agent hp
    agent_1->reset();
    agent_2->reset();

    // Reset hill
    hill->reset();
}
}