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
      hidden_state(torch::zeros({1, 64})),
      io(&io),
      last_action_time(0),
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

    center_camera_on_body();
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
    auto action = checkpoint_selector_window.update(policy,
                                                    agent->get_observation().size(),
                                                    agent->get_actions().size());
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

void TrainingWizardScreen::center_camera_on_body()
{
    glm::vec2 size{19.2f, 10.8f};
    glm::vec2 half_size = size * 0.5f;
    glm::vec2 offset{(size.x * 0.333) - half_size.x, (size.y * 0.5) - half_size.y};
    b2Vec2 agent_position;
    if (agent->get_modules().size() > 0)
    {
        agent_position = agent->get_rigid_body()->body->GetTransform().p;
    }
    else
    {
        agent_position = {0, 0};
    }
    glm::vec2 center{agent_position.x + offset.x, agent_position.y + offset.y};
    projection = glm::ortho(center.x - half_size.x,
                            center.x + half_size.x,
                            center.y - half_size.y,
                            center.y + half_size.y);
}

void TrainingWizardScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();

    if (agent->get_modules().size() > 0)
    {
        auto render_data = agent->get_render_data();
        renderer.draw(render_data, projection, elapsed_time);
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

    if (!policy.is_empty() && elapsed_time - last_action_time >= 0.1)
    {
        last_action_time = elapsed_time;
        auto hidden_state_size = policy->get_hidden_size();
        if (hidden_state.size(0) != hidden_state_size)
        {
            hidden_state = torch::zeros({1, hidden_state_size});
        }
        auto observation_vec = agent->get_observation();
        auto observation = torch::from_blob(observation_vec.data(), {1, static_cast<long>(observation_vec.size())});
        auto mask = torch::ones({1, 1});
        auto act_result = policy->act(observation, hidden_state, mask);
        auto actions_tensor = act_result[1][0].to(torch::kInt).contiguous();
        std::vector<int> actions(actions_tensor.data<int>(), actions_tensor.data<int>() + actions_tensor.numel());
        agent->act(actions);
    }

    if (agent->get_modules().size() > 0)
    {
        center_camera_on_body();
    }
}
}