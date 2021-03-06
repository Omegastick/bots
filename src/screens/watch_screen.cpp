#include <algorithm>
#include <memory>
#include <filesystem>
#include <future>

#include <cpprl/cpprl.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <fmt/format.h>

#include "screens/watch_screen.h"
#include "graphics/renderers/renderer.h"
#include "graphics/post_processing/post_proc_layer.h"
#include "training/environments/ienvironment.h"
#include "misc/resource_manager.h"

namespace fs = std::filesystem;

namespace ai
{
WatchScreen::WatchScreen(std::unique_ptr<IEnvironment> environment, ResourceManager &resource_manager, IO &io)
    : checkpoint_selector_window(io),
      environment(std::move(environment)),
      policy(nullptr),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f)),
      resource_manager(&resource_manager),
      state(States::BROWSING),
      frame_counter(0),
      scores({{0}})
{
    resource_manager.load_texture("bullet", "images/bullet.png");
    resource_manager.load_texture("pixel", "images/pixel.png");
    resource_manager.load_texture("target", "images/target.png");
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");
    resource_manager.load_font("roboto-16", "fonts/Roboto-Regular.ttf", 16);
}

WatchScreen::~WatchScreen() {}

void WatchScreen::update(const double /*delta_time*/)
{
    if (state == States::BROWSING)
    {
        auto policy_ = checkpoint_selector_window.update();
        if (policy_ != nullptr)
        {
            policy = *policy_;
            environment->start_thread();
            observations = environment->reset().get().observation;
            masks = torch::zeros({1, 1});
            hidden_states = torch::zeros({1, 24});

            state = States::WATCHING;
        }
    }
    else if (state == States::WATCHING)
    {
        if (++frame_counter >= 6)
        {
            frame_counter = 0;
            action_update();
        }
        else
        {
            environment->forward(1.f / 60.f);
        }

        show_agent_scores();
    }
}

void WatchScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    if (state == States::WATCHING)
    {
        renderer.scissor(-10, -20, 10, 20, glm::ortho(-38.4f, 38.4f, -21.6f, 21.6f));
        environment->draw(renderer);
    }
}

void WatchScreen::action_update()
{
    // Get actions from policy
    std::vector<torch::Tensor> act_result;
    {
        torch::NoGradGuard no_grad;
        act_result = policy->act(observations,
                                 hidden_states,
                                 masks);
    }
    hidden_states = act_result[3];

    // Step environment
    auto step_info = environment->step({act_result[1][0], act_result[1][1]},
                                       1.f / 60.f)
                         .get();
    observations = step_info.observation.view({1, -1});
    for (unsigned int i = 0; i < scores.size(); ++i)
    {
        scores[i].push_back(scores[i].back() + step_info.reward[i].item().toFloat());
    }
    if (step_info.done[0].item().toBool())
    {
        for (auto &agent_scores : scores)
        {
            agent_scores = {0};
        }
    }
}

void WatchScreen::show_checkpoint_selector()
{
}

void WatchScreen::show_agent_scores()
{
    for (unsigned int i = 0; i < scores.size(); ++i)
    {
        std::string title = fmt::format("Agent {}", i + 1);
        ImGui::Begin(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
        ImGui::Text("Score: %.0f", scores[0].back());
        ImGui::PlotLines("", &scores[0].front(), scores[0].size());
        ImGui::End();
    }
}
}