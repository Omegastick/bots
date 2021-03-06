#include <chrono>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <torch/torch.h>

#include "game.h"
#include "audio/audio_engine.h"
#include "misc/module_factory.h"
#include "misc/random.h"
#include "training/entities/bullet.h"
#include "training/entities/ientity.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "training/bodies/body.h"
#include "training/bodies/test_body.h"

namespace ai
{
Game::Game(double tick_length,
           BodyFactory &body_factory,
           IEnvironmentFactory &env_factory,
           Random &rng)
    : body_factory(body_factory),
      current_tick(0),
      env_factory(env_factory),
      last_tick_time(0),
      rng(rng),
      tick_length(tick_length) {}

bool Game::add_body(nlohmann::json body_spec)
{
    body_specs.push_back(body_spec);

    if (static_cast<int>(body_specs.size()) == env_factory.get_num_bodies())
    {
        setup_env();
        return true;
    }
    return false;
}

bool Game::ready_to_tick(double current_time)
{
    return current_time - last_tick_time >= tick_length && env != nullptr;
}

void Game::set_action(int tick, int player, const std::vector<int> &action)
{
    action_store->add_action(tick, player, action);
}

void Game::setup_env()
{
    auto b2_world = std::make_unique<b2World>(b2Vec2{0, 0});
    std::vector<std::unique_ptr<Body>> bodies;
    std::vector<int> actions_per_player;
    std::transform(body_specs.begin(), body_specs.end(),
                   std::back_inserter(bodies),
                   [&](const nlohmann::json &json) {
                       auto body = body_factory.make(*b2_world, rng);
                       body->load_json(json);
                       actions_per_player.push_back(json["num_actions"]);
                       return body;
                   });

    env = env_factory.make(std::make_unique<Random>(rng.next_int(0, 65535)),
                           std::move(b2_world),
                           std::move(bodies),
                           RewardConfig());
    env->set_audibility(false);
    env->reset();

    action_store = std::make_unique<ActionStore>(actions_per_player);
}

TickResult Game::tick(double current_time)
{
    if (env == nullptr)
    {
        throw std::runtime_error("Environment not set up yet");
    }

    last_tick_time = current_time;

    auto actions = action_store->get_actions(current_tick);
    std::vector<torch::Tensor> actions_tensors;
    std::transform(actions.begin(), actions.end(),
                   std::back_inserter(actions_tensors),
                   [](std::vector<int> &actions_vec) {
                       return torch::from_blob(actions_vec.data(),
                                               {static_cast<long>(actions_vec.size())},
                                               torch::kInt);
                   });
    for (int i = 0; i < 5; ++i)
    {
        env->forward(1.f / 60.f);
        env->clear_effects();
    }
    auto step_info = env->step(actions_tensors, 1.f / 60.f);

    ++current_tick;

    std::unordered_map<unsigned int, Transform> entity_transforms;
    for (const auto &entity : env->get_entities())
    {
        auto b2_transform = entity.second->get_transform();
        entity_transforms[entity.first] = {b2_transform.p.x,
                                           b2_transform.p.y,
                                           b2_transform.q.GetAngle()};
    }

    std::vector<Transform> agent_transforms;
    for (const auto &body : env->get_bodies())
    {
        auto b2_transform = body->get_rigid_body().body->GetTransform();
        agent_transforms.push_back({b2_transform.p.x,
                                    b2_transform.p.y,
                                    b2_transform.q.GetAngle()});
    }

    std::vector<float> hps;
    for (const auto &body : env->get_bodies())
    {
        hps.push_back(body->get_hp());
    }

    return {std::move(agent_transforms),
            std::move(entity_transforms),
            std::move(step_info.events),
            std::move(hps),
            env->get_scores(),
            step_info.done[0].item().toBool(),
            current_tick,
            step_info.victor};
}

TEST_CASE("Game")
{
    Random rng(0);
    MockAudioEngine audio_engine;
    BulletFactory bullet_factory(audio_engine);
    ModuleFactory module_factory(audio_engine, bullet_factory, rng);
    TestBodyFactory body_factory(module_factory, rng);
    KothEnvFactory env_factory(10, audio_engine, body_factory, bullet_factory);
    Game game(0.1, body_factory, env_factory, rng);

    SUBCASE("add_body() returns true after enough bodies are added")
    {
        TestBody test_body(module_factory, rng);
        auto body_spec = test_body.to_json();

        DOCTEST_CHECK(game.add_body(body_spec) == false);
        DOCTEST_CHECK(game.add_body(body_spec) == true);
    }

    SUBCASE("ready_to_tick()")
    {
        TestBody test_body(module_factory, rng);
        auto body_spec = test_body.to_json();

        SUBCASE("Returns false if bodies have not been added")
        {
            DOCTEST_CHECK(game.ready_to_tick(100) == false);
            game.add_body(body_spec);
            DOCTEST_CHECK(game.ready_to_tick(100) == false);
            game.add_body(body_spec);
        }

        SUBCASE("Returns false if called before tick is ready")
        {
            game.add_body(body_spec);
            game.add_body(body_spec);

            DOCTEST_CHECK(game.ready_to_tick(0) == false);
            DOCTEST_CHECK(game.ready_to_tick(0.05) == false);
            DOCTEST_CHECK(game.ready_to_tick(0.09999999) == false);
        }

        SUBCASE("Returns true if called after tick is ready")
        {
            game.add_body(body_spec);
            game.add_body(body_spec);

            DOCTEST_CHECK(game.ready_to_tick(0.1) == true);
            DOCTEST_CHECK(game.ready_to_tick(100) == true);
        }
    }

    SUBCASE("tick()")
    {
        SUBCASE("Throws when called without enough bodies")
        {
            DOCTEST_CHECK_THROWS(game.tick(0));

            TestBody test_body(module_factory, rng);
            auto body_spec = test_body.to_json();
            game.add_body(body_spec);

            DOCTEST_CHECK_THROWS(game.tick(0));
        }

        SUBCASE("Returns a finished result within the maximum amount of time steps")
        {
            TestBody test_body(module_factory, rng);
            auto body_spec = test_body.to_json();

            game.add_body(body_spec);
            game.add_body(body_spec);

            bool finished = false;
            for (int i = 0; i < 10; ++i)
            {
                game.set_action(i, 0, {0, 0, 0, 0});
                game.set_action(i, 1, {0, 0, 0, 0});
                auto result = game.tick(0);
                finished |= result.done;
            }
        }
    }
}
}