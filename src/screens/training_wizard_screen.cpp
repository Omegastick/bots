#include <memory>

#include <imgui.h>
#include <nlohmann/json.hpp>

#include "screens/training_wizard_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/sprite.h"
#include "misc/io.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "training/agents/agent.h"
#include "ui/training_wizard_screen/body_selector_window.h"

namespace SingularityTrainer
{
TrainingWizardScreen::TrainingWizardScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, Random &rng, IO &io)
    : agent(std::make_unique<Agent>()),
      b2_world({0, 0}),
      body_selector_window(io),
      io(&io),
      resource_manager(&resource_manager),
      rng(&rng),
      screen_manager(&screen_manager),
      state(State::Body)
{
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
    if (body_selector_window.update(*rng, b2_world, *agent))
    {
        state = State::Checkpoint;
    }

    if (agent != nullptr)
    {
        ImGui::SetNextWindowSize({0, 0});
        ImGui::Begin("Output");
        ImGui::Text("%s", agent->to_json().dump(4).c_str());
        ImGui::End();
    }
}
void TrainingWizardScreen::checkpoint()
{
    ImGui::Begin("Checkpoint", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Checkpoint");
    if (ImGui::Button("Next"))
    {
        state = State::Algorithm;
    }
    ImGui::End();
}

void TrainingWizardScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.begin();
    renderer.end();
}

void TrainingWizardScreen::update(double /*delta_time*/)
{
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
}
}