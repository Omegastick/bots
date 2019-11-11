#include <memory>

#include <Box2D/Box2D.h>
#include <cpprl/cpprl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <torch/torch.h>
#include <tweeny.h>

#include "screens/training_wizard_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/render_data.h"
#include "misc/animator.h"
#include "misc/io.h"
#include "misc/random.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "training/bodies/body.h"
#include "ui/training_wizard_screen/body_selector_window.h"
#include "ui/training_wizard_screen/wizard_action.h"

namespace SingularityTrainer
{
TrainingWizardScreen::TrainingWizardScreen(std::unique_ptr<Body> body,
                                           std::unique_ptr<b2World> world,
                                           Animator &animator,
                                           TrainingProgram &program,
                                           ResourceManager &resource_manager,
                                           ScreenManager &screen_manager,
                                           IO &io)
    : body(std::move(body)),
      animator(animator),
      body_selector_window(io),
      checkpoint_selector_window(io),
      elapsed_time(0),
      hidden_state(torch::zeros({1, 64})),
      io(&io),
      last_action_time(0),
      policy(nullptr),
      program(program),
      resource_manager(&resource_manager),
      screen_manager(&screen_manager),
      state(State::Body),
      world(std::move(world)),
      x_offset(0.333)
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

    crt_post_proc_layer = PostProcLayer(*resource_manager.shader_store.get("crt"),
                                        io.get_resolution().x,
                                        io.get_resolution().y);

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
    auto action = body_selector_window.update(*body, program);
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
                                                    body->get_observation().size(),
                                                    body->get_actions().size(),
                                                    program);
    if (action == WizardAction::Next)
    {
        auto tween = std::make_shared<tweeny::tween<double>>(tweeny::from(x_offset)
                                                                 .to(0.5)
                                                                 .during(1000)
                                                                 .via(tweeny::easing::sinusoidalInOut));
        animator.add_animation({[this, tween](float step_percentage) {
                                    this->x_offset = tween->step(step_percentage);
                                },
                                3});
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
    glm::vec2 offset{(size.x * x_offset) - half_size.x, (size.y * 0.5) - half_size.y};
    b2Vec2 body_position;
    if (body->get_modules().size() > 0)
    {
        body_position = body->get_rigid_body().body->GetTransform().p;
    }
    else
    {
        body_position = {0, 0};
    }
    glm::vec2 center{body_position.x + offset.x, body_position.y + offset.y};
    projection = glm::ortho(center.x - half_size.x,
                            center.x + half_size.x,
                            center.y - half_size.y,
                            center.y + half_size.y);
}

void TrainingWizardScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.push_post_proc_layer(crt_post_proc_layer);

    if (body->get_modules().size() > 0)
    {
        body->draw(renderer);
    }

    auto crt_shader = resource_manager->shader_store.get("crt");
    crt_shader.set_uniform_2f("u_resolution", {renderer.get_width(), renderer.get_height()});
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

    world->Step(delta_time, 3, 3);

    if (!policy.is_empty() && elapsed_time - last_action_time >= 0.1)
    {
        last_action_time = elapsed_time;
        auto hidden_state_size = policy->get_hidden_size();
        if (hidden_state.size(0) != hidden_state_size)
        {
            hidden_state = torch::zeros({1, hidden_state_size});
        }
        auto observation_vec = body->get_observation();
        auto observation = torch::from_blob(observation_vec.data(), {1, static_cast<long>(observation_vec.size())});
        auto mask = torch::ones({1, 1});
        auto act_result = policy->act(observation, hidden_state, mask);
        auto actions_tensor = act_result[1][0].to(torch::kInt).contiguous();
        std::vector<int> actions(actions_tensor.data_ptr<int>(), actions_tensor.data_ptr<int>() + actions_tensor.numel());
        body->act(actions);
    }

    if (body->get_modules().size() > 0)
    {
        center_camera_on_body();
        body->get_rigid_body().body->SetLinearVelocity({0, 0});
    }
}
}