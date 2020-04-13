#include <future>
#include <memory>
#include <vector>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "ecs_env.h"
#include "audio/audio_engine.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/components/bullet.h"
#include "environment/components/done.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/physics_body.h"
#include "environment/components/score.h"
#include "environment/observers/destroy_physics_body.h"
#include "environment/serialization/serialize_body.h"
#include "environment/systems/action_system.h"
#include "environment/systems/audio_system.h"
#include "environment/systems/body_death_system.h"
#include "environment/systems/clean_up_system.h"
#include "environment/systems/distortion_system.h"
#include "environment/systems/health_bar_system.h"
#include "environment/systems/hill_system.h"
#include "environment/systems/modules/gun_module_system.h"
#include "environment/systems/modules/laser_sensor_module_system.h"
#include "environment/systems/modules/thruster_module_system.h"
#include "environment/systems/module_system.h"
#include "environment/systems/new_frame_system.h"
#include "environment/systems/observation_system.h"
#include "environment/systems/particle_system.h"
#include "environment/systems/physics_system.h"
#include "environment/systems/render_system.h"
#include "environment/systems/trail_system.h"
#include "environment/utils/body_utils.h"
#include "environment/utils/hill_utils.h"
#include "environment/utils/wall_utils.h"
#include "graphics/renderers/renderer.h"
#include "misc/transform.h"

namespace ai
{
EcsEnv::EcsEnv(double game_length)
    : audible(true),
      bodies{entt::null, entt::null},
      elapsed_time(0),
      game_length(game_length)
{
    registry.set<Done>(false);

    init_physics(registry);

    make_wall(registry, {0.f, -20.f}, {20.f, 0.1f}, 0.f);
    make_wall(registry, {0.f, 20.f}, {20.f, 0.1f}, 0.f);
    make_wall(registry, {-10.f, 0.f}, {0.1f, 40.1f}, 0.f);
    make_wall(registry, {10.f, 0.f}, {0.1f, 40.1f}, 0.f);
    make_wall(registry, {0, -10.f}, {5.f, 0.2f}, 0.f);
    make_wall(registry, {0, 10.f}, {5.f, 0.2f}, 0.f);

    make_hill(registry, {0.f, 0.f}, 3.f);

    const auto background_entity = registry.create();
    registry.emplace<EcsRectangle>(background_entity);
    registry.emplace<Color>(background_entity, set_alpha(cl_base03, 0.95f), glm::vec4{0, 0, 0, 0});
    auto &background_transform = registry.emplace<Transform>(background_entity);
    background_transform.set_scale({20.f, 40.f});
    background_transform.set_z(-5);
}

void EcsEnv::draw(Renderer &renderer, IAudioEngine &audio_engine, bool lightweight)
{
    const double view_height = 50;
    auto view_top = view_height * 0.5;
    auto view_right = view_top * (static_cast<float>(renderer.get_width()) /
                                  static_cast<float>(renderer.get_height()));
    const auto view = glm::ortho(-view_right, view_right, -view_top, view_top);
    renderer.set_view(view);

    if (!lightweight)
    {
        trail_system(registry);
        draw_lasers_system(registry);
        particle_system(registry, renderer);
        distortion_system(registry, renderer);
        audio_system(registry, audio_engine);
    }

    health_bar_system(registry);
    render_system(registry, renderer);
    // debug_render_system(registry, renderer);
}

void EcsEnv::forward(double step_length)
{
    physics_system(registry, step_length);
    module_system(registry);
    thruster_particle_system(registry);
    clean_up_system(registry);
    elapsed_time += step_length;
}

double EcsEnv::get_elapsed_time() const
{
    return elapsed_time;
}

std::pair<double, double> EcsEnv::get_scores() const
{
    if (registry.valid(bodies[0]) && registry.valid(bodies[1]))
    {
        return {registry.get<Score>(bodies[0]).score, registry.get<Score>(bodies[1]).score};
    }
    return {0, 0};
}

bool EcsEnv::is_audible() const
{
    return audible;
}

EcsStepInfo EcsEnv::reset()
{
    if (bodies[0] != entt::null)
    {
        auto &physics_body = registry.get<PhysicsBody>(bodies[0]);
        physics_body.body->SetTransform({0.f, -15.f}, 0.f);
        physics_body.body->SetLinearVelocity(b2Vec2_zero);
        physics_body.body->SetAngularVelocity(0.f);
        registry.get<Score>(bodies[0]).score = 0.f;
    }
    if (bodies[1] != entt::null)
    {
        auto &physics_body = registry.get<PhysicsBody>(bodies[1]);
        physics_body.body->SetTransform({0.f, 15.f}, glm::radians(180.f));
        physics_body.body->SetLinearVelocity(b2Vec2_zero);
        physics_body.body->SetAngularVelocity(0.f);
        registry.get<Score>(bodies[1]).score = 0.f;
    }

    elapsed_time = 0;
    registry.set<Done>(false);

    const auto bullets = registry.view<EcsBullet>();
    registry.destroy(bullets.begin(), bullets.end());

    reset_hill(registry);
    clean_up_system(registry);

    return {observation_system(registry), torch::zeros({2, 1}), torch::zeros({2, 1})};
}

void EcsEnv::set_audibility(bool audibility)
{
    audible = audibility;
}

void EcsEnv::set_body(std::size_t index, const nlohmann::json &body_def)
{
    if (bodies[index] != entt::null)
    {
        destroy_body(registry, bodies[index]);
        clean_up_system(registry);
    }

    bodies[index] = deserialize_body(registry, body_def);
}

EcsStepInfo EcsEnv::step(const std::vector<torch::Tensor> &actions, double step_length)
{
    new_frame_system(registry);

    action_system(registry, actions, bodies.data(), bodies.size());
    gun_module_system(registry);
    thruster_module_system(registry);

    forward(step_length);

    hill_system(registry);
    laser_sensor_module_system(registry);

    body_death_system(registry, bodies.data(), bodies.size());

    registry.set<Done>(registry.ctx<Done>().done || elapsed_time >= game_length);

    if (registry.ctx<Done>().done)
    {
        int winner;
        const auto &score_0 = registry.get<Score>(bodies[0]).score;
        const auto &score_1 = registry.get<Score>(bodies[1]).score;
        if (score_0 == score_1)
        {
            winner = -1;
        }
        else if (score_0 > score_1)
        {
            winner = 0;
        }
        else
        {
            winner = 1;
        }
        return {observation_system(registry), torch::zeros({2, 1}), torch::ones({2, 1}), winner};
    }
    return {observation_system(registry), torch::zeros({2, 1}), torch::zeros({2, 1})};
}

TEST_CASE("EcsEnv")
{
    SUBCASE("Runs a game without errors")
    {
        EcsEnv env(1);

        env.set_body(0, default_body());
        env.set_body(1, default_body());

        auto step_info = env.reset();

        while (!step_info.done[0].item().toBool())
        {
            step_info = env.step({torch::rand({1, 4}), torch::rand({1, 4})}, 1.f / 60.f);
            for (int i = 0; i < 5; i++)
            {
                env.forward(1.f / 60.f);
            }
        }
    }

    SUBCASE("Runs multiple games in parallel")
    {
        std::vector<std::future<void>> futures;

        for (int i = 0; i < 8; i++)
        {
            futures.emplace_back(std::async(std::launch::async, [] {
                EcsEnv env(1);

                env.set_body(0, default_body());
                env.set_body(1, default_body());

                auto step_info = env.reset();

                while (!step_info.done[0].item().toBool())
                {
                    step_info = env.step({torch::rand({1, 4}), torch::rand({1, 4})}, 1.f / 60.f);
                    for (int i = 0; i < 5; i++)
                    {
                        env.forward(1.f / 60.f);
                    }
                }
            }));
        }

        for (auto &future : futures)
        {
            future.get();
        }
    }
}
}