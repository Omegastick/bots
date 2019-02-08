#include "test_screen/test_env.h"
#include "graphics/colors.h"

namespace SingularityTrainer
{
class TestContactListener : public b2ContactListener
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
                case RigidBody::ParentTypes::Bot:
                    static_cast<Bot *>(body->parent)->begin_contact(other);
                    break;
                case RigidBody::ParentTypes::Target:
                    static_cast<Target *>(body->parent)->begin_contact(other);
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
                case RigidBody::ParentTypes::Bot:
                    static_cast<Bot *>(body->parent)->end_contact(other);
                    break;
                case RigidBody::ParentTypes::Target:
                    static_cast<Target *>(body->parent)->end_contact(other);
                    break;
                }
            }
        }
    }
};

TestEnv::TestEnv(ResourceManager &resource_manager, float x, float y, float scale, int max_steps)
    : max_steps(max_steps),
      command_queue_flag(0),
      reward(0),
      step_counter(0),
      done(false),
      score_display(-4.8, -4.8, 0.999, resource_manager)
{
    // Box2D world
    world = std::make_unique<b2World>(b2Vec2(0, 0));
    contact_listener = std::make_unique<TestContactListener>();
    world->SetContactListener(contact_listener.get());

    // Bot
    bot = std::make_unique<Bot>(resource_manager, *world);

    // Target
    target = std::make_unique<Target>(4, 4, *world, *this);

    // Walls
    walls.push_back(std::make_unique<Wall>(-5, -5, 10, 0.1, *world));
    walls.push_back(std::make_unique<Wall>(-5, -5, 0.1, 10, *world));
    walls.push_back(std::make_unique<Wall>(-5, 4.9, 10, 0.1, *world));
    walls.push_back(std::make_unique<Wall>(4.9, -5, 0.1, 10, *world));

    // Display
    render_texture.create(1000, 1000);
    sprite.setTexture(render_texture.getTexture());
    sprite.setPosition(x, y);
    sprite.setScale(scale, scale);
    render_texture.setView(sf::View(sf::Vector2f(0, 0), sf::Vector2f(10, 10)));
}

TestEnv::~TestEnv()
{
    ThreadCommand command{Commands::Quit};
    command_queue.emplace(std::move(command));
    command_queue_flag++;
    thread->join();
};

void TestEnv::start_thread()
{
    thread = new std::thread(&TestEnv::thread_loop, this);
}

void TestEnv::draw(sf::RenderTarget &render_target, bool lightweight)
{
    // Draw onto temporary texture
    render_texture.clear(cl_background);
    bot->draw(render_texture, lightweight);
    for (auto &wall : walls)
    {
        wall->draw(render_texture, lightweight);
    }
    target->draw(render_texture, lightweight);
    score_display.draw(render_texture, lightweight);
    render_texture.display();

    // Draw temporary tecture onto window
    sprite.setTexture(render_texture.getTexture());
    render_target.draw(sprite);
}

std::future<std::unique_ptr<StepInfo>> TestEnv::step(std::vector<int> &actions, float step_length)
{
    std::promise<std::unique_ptr<StepInfo>> promise;
    std::future<std::unique_ptr<StepInfo>> future = promise.get_future();
    ThreadCommand command{Commands::Step, std::move(promise), step_length, actions};
    command_queue.emplace(std::move(command));
    command_queue_flag++;
    return future;
}

void TestEnv::forward(float step_length)
{
    std::promise<std::unique_ptr<StepInfo>> promise;
    ThreadCommand command{Commands::Forward, std::move(promise), step_length};
    command_queue.emplace(std::move(command));
    command_queue_flag++;
}

void TestEnv::change_reward(float reward_delta)
{
    reward += reward_delta;
}

void TestEnv::set_done()
{
    done = true;
}

std::future<std::unique_ptr<StepInfo>> TestEnv::reset()
{
    std::promise<std::unique_ptr<StepInfo>> promise;
    std::future<std::unique_ptr<StepInfo>> future = promise.get_future();
    ThreadCommand command{Commands::Reset, std::move(promise)};
    command_queue.emplace(std::move(command));
    command_queue_flag++;
    return future;
}

void TestEnv::thread_loop()
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
        case Commands::Quit:
            quit = true;
            break;
        case Commands::Step:
            // Act
            bot->act(command.actions);

            // Step simulation
            world->Step(command.step_length, 3, 2);

            // Max episode length
            step_counter++;
            done = step_counter >= max_steps;

            // Return step information
            step_info->observation = bot->get_observation();
            step_info->reward = reward;
            step_info->done = done;

            score_display.add_score(reward);

            // Reset reward
            reward = 0;

            if (done)
            {
                reset();
            }

            // Return
            command.promise.set_value(std::move(step_info));
            break;
        case Commands::Forward:
            world->Step(command.step_length, 3, 2);
            break;
        case Commands::Reset:
            done = false;
            reward = 0;
            step_counter = 0;

            // Reset bot position
            bot->rigid_body->body->SetTransform(b2Vec2_zero, 0);
            bot->rigid_body->body->SetAngularVelocity(0);
            bot->rigid_body->body->SetLinearVelocity(b2Vec2_zero);

            // Fill in StepInfo
            step_info->observation = bot->get_observation();
            step_info->done = done;
            step_info->reward = reward;

            // Return
            command.promise.set_value(std::move(step_info));
            break;
        }
    }
}
}