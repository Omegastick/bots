#include <memory>

#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "ecs_env.h"
#include "audio/audio_engine.h"
#include "environment/components/activatable.h"
#include "environment/components/body.h"
#include "environment/components/ecs_render_data.h"
#include "environment/components/particle_emitter.h"
#include "environment/components/physics_body.h"
#include "environment/serialization/serialize_body.h"
#include "environment/systems/audio_system.h"
#include "environment/systems/clean_up_system.h"
#include "environment/systems/distortion_system.h"
#include "environment/systems/health_bar_system.h"
#include "environment/systems/hill_system.h"
#include "environment/systems/modules/gun_module_system.h"
#include "environment/systems/modules/laser_sensor_module_system.h"
#include "environment/systems/modules/thruster_module_system.h"
#include "environment/systems/module_system.h"
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
EcsEnv::EcsEnv()
    : audible(true),
      bodies{entt::null, entt::null}
{
    init_physics(registry);

    make_wall(registry, {0.f, -20.f}, {20.f, 0.1f}, 0.f);
    make_wall(registry, {0.f, 20.f}, {20.f, 0.1f}, 0.f);
    make_wall(registry, {-10.f, 0.f}, {0.1f, 40.1f}, 0.f);
    make_wall(registry, {10.f, 0.f}, {0.1f, 40.1f}, 0.f);
    make_wall(registry, {0, -10.f}, {5.f, 0.2f}, 0.f);
    make_wall(registry, {0, 10.f}, {5.f, 0.2f}, 0.f);

    make_hill(registry, {0.f, 0.f}, 3.f);

    const auto background_entity = registry.create();
    registry.assign<EcsRectangle>(background_entity);
    registry.assign<Color>(background_entity, set_alpha(cl_base03, 0.95f), glm::vec4{0, 0, 0, 0});
    auto &background_transform = registry.assign<Transform>(background_entity);
    background_transform.set_scale({20.f, 40.f});
    background_transform.set_z(-5);
}

void EcsEnv::draw(Renderer &renderer, IAudioEngine &audio_engine, bool /*lightweight*/)
{
    const double view_height = 50;
    auto view_top = view_height * 0.5;
    auto view_right = view_top * (static_cast<float>(renderer.get_width()) /
                                  static_cast<float>(renderer.get_height()));
    const auto view = glm::ortho(-view_right, view_right, -view_top, view_top);
    renderer.set_view(view);

    health_bar_system(registry);
    trail_system(registry);
    draw_lasers_system(registry);
    particle_system(registry, renderer);
    distortion_system(registry, renderer);
    render_system(registry, renderer);
    audio_system(registry, audio_engine);
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

bool EcsEnv::is_audible() const
{
    return audible;
}

EcsStepInfo EcsEnv::reset()
{
    return {};
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

EcsStepInfo EcsEnv::step(std::vector<torch::Tensor> /*actions*/, double step_length)
{
    forward(step_length);
    gun_module_system(registry);
    thruster_module_system(registry);
    laser_sensor_module_system(registry);
    hill_system(registry);

    return {};
}
}