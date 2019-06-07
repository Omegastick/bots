#include <memory>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "create_program_screen.h"
#include "graphics/renderers/renderer.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "training/agents/agent.h"
#include "training/environments/ienvironment.h"
#include "training/environments/koth_env.h"
#include "ui/create_program_screen/body_selector_window.h"
#include "ui/create_program_screen/tabs.h"

namespace SingularityTrainer
{

CreateProgramScreen::CreateProgramScreen(std::unique_ptr<BodySelectorWindow> body_selector_window,
                                         std::unique_ptr<IEnvironment> environment,
                                         std::unique_ptr<TrainingProgram> program,
                                         std::unique_ptr<Tabs> tabs,
                                         IO &io,
                                         ResourceManager &resource_manager,
                                         ScreenManager &screen_manager)
    : body_selector_window(std::move(body_selector_window)),
      environment(std::move(environment)),
      io(io),
      program(std::move(program)),
      resource_manager(resource_manager),
      screen_manager(screen_manager),
      state(CreateProgramScreenState::Body),
      tabs(std::move(tabs))
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("crt", "shaders/texture.vert", "shaders/crt.frag");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);

    crt_post_proc_layer = PostProcLayer(resource_manager.shader_store.get("crt").get(),
                                        io.get_resolution().x,
                                        io.get_resolution().y);

    this->environment->start_thread();
}

void CreateProgramScreen::algorithm()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Algorithm");
    ImGui::End();
}

void CreateProgramScreen::body()
{
    auto json = body_selector_window->update();
    if (!json.empty())
    {
        auto agents = environment->get_agents();
        agents[0]->load_json(json);
        agents[1]->load_json(json);
        environment->reset();
    }
}

void CreateProgramScreen::checkpoint()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Checkpoint");
    ImGui::End();
}

void CreateProgramScreen::rewards()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Rewards");
    ImGui::End();
}

void CreateProgramScreen::save_load()
{
    ImGui::SetNextWindowSize({0, 0});
    ImGui::Begin("Save/load");
    ImGui::End();
}

void CreateProgramScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.push_post_proc_layer(&crt_post_proc_layer);
    renderer.begin();

    auto render_data = environment->get_render_data(lightweight);

    const double view_height = 50;
    auto view_top = view_height * 0.5;
    auto view_right = view_top * 1.777777;
    renderer.draw(render_data, glm::ortho(-view_right, view_right, -view_top, view_top), 0, lightweight);

    auto crt_shader = resource_manager.shader_store.get("crt");
    crt_shader->set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
    crt_shader->set_uniform_1f("u_output_gamma", 1);
    crt_shader->set_uniform_1f("u_strength", 0.3);
    crt_shader->set_uniform_1f("u_distortion_factor", 0.05);

    renderer.end();
}

void CreateProgramScreen::update(double /*delta_time*/)
{
    state = tabs->update();

    switch (state)
    {
    case Algorithm:
        algorithm();
        break;
    case Body:
        body();
        break;
    case Checkpoint:
        checkpoint();
        break;
    case Rewards:
        rewards();
        break;
    case SaveLoad:
        save_load();
        break;
    case Opponents:
    case Schedule:
        break;
    }
}

std::shared_ptr<IScreen> CreateProgramScreenFactory::make()
{
    auto world = std::make_unique<b2World>(b2Vec2_zero);
    auto rng = std::make_unique<Random>(0);
    std::vector<std::unique_ptr<Agent>> agents;
    agents.push_back(agent_factory.make(*world, *rng));
    agents.push_back(agent_factory.make(*world, *rng));
    auto environment = env_factory.make(std::move(rng), std::move(world), std::move(agents));
    return std::make_shared<CreateProgramScreen>(std::make_unique<BodySelectorWindow>(io),
                                                 std::move(environment),
                                                 std::make_unique<TrainingProgram>(),
                                                 std::make_unique<Tabs>(io),
                                                 io,
                                                 resource_manager,
                                                 screen_manager);
}
}