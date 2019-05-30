#include <memory>

#include <cpprl/cpprl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <torch/torch.h>

#include "screens/training_wizard_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/sprite.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "training/agents/agent.h"
#include "ui/training_wizard_screen/body_selector_window.h"
#include "ui/training_wizard_screen/wizard_action.h"

namespace SingularityTrainer
{
TrainingWizardScreen::TrainingWizardScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, Random &rng, IO &io)
    : agent(std::make_unique<Agent>()),
      b2_world({0, 0}),
      body_selector_window(io),
      checkpoint_selector_window(io),
      elapsed_time(0),
      io(&io),
      policy(nullptr),
      resource_manager(&resource_manager),
      rng(&rng),
      screen_manager(&screen_manager),
      state(State::Body)
{
    resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);

    glm::vec2 size{19.2f, 10.8f};
    glm::vec2 half_size = size * 0.5f;
    glm::vec2 center = {(size.x * 0.333) - half_size.x, (size.y * 0.5) - half_size.y};
    projection = glm::ortho(center.x - half_size.x,
                            center.x + half_size.x,
                            center.y - half_size.y,
                            center.y + half_size.y);
}

void TrainingWizardScreen::algorithm()
{
    ImGui::Begin("Algorithm", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Algorithm");
    if (ImGui::Button("Next"))
    {
        screen_manager->close_screen();
    }
    ImGui::End();
}

void TrainingWizardScreen::body()
{
    auto action = body_selector_window.update(*rng, b2_world, *agent);
    if (action == WizardAction::Next)
    {
        state = State::Checkpoint;
    }
    else if (action == WizardAction::Cancel)
    {
        screen_manager->close_screen();
    }
}

void TrainingWizardScreen::checkpoint()
{
    auto action = checkpoint_selector_window.update(policy);
    if (action == WizardAction::Next)
    {
        state = State::Algorithm;
    }
    else if (action == WizardAction::Back)
    {
        state = State::Body;
    }
    else if (action == WizardAction::Cancel)
    {
        screen_manager->close_screen();
    }
}

void TrainingWizardScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();

    if (agent->get_modules().size() > 0)
    {
        auto render_data = agent->get_render_data();
        renderer.draw(render_data, projection, 0);
    }

    renderer.end();
}

void TrainingWizardScreen::update(double delta_time)
{
    elapsed_time += delta_time;

    switch (state)
    {
    case Body:
        body();
        break;
    case Checkpoint:
        checkpoint();
        break;
    case Algorithm:
        algorithm();
        break;
    }

    b2_world.Step(delta_time, 3, 3);

    if (agent->get_modules().size() > 0 && elapsed_time >= 0.1)
    {
        elapsed_time = 0;
        auto num_actions = agent->get_actions().size();
        std::vector<int> actions;
        for (unsigned int i = 0; i < num_actions; ++i)
        {
            actions.push_back(rng->next_int(0, 2));
        }
        agent->act(actions);
    }
}
}