#pragma once

#include <memory>

#include <cpprl/model/policy.h>
#include <Box2D/Box2D.h>
#include <glm/mat4x4.hpp>
#include <torch/torch.h>

#include "screens/iscreen.h"
#include "training/training_program.h"
#include "ui/training_wizard_screen/body_selector_window.h"
#include "ui/training_wizard_screen/wizard_checkpoint_selector_window.h"

namespace SingularityTrainer
{
class Agent;
class IO;
class Random;
class Renderer;
class ResourceManager;
class ScreenManager;

class TrainingWizardScreen : public IScreen
{
  private:
    enum State
    {
        Body,
        Checkpoint,
        Algorithm,
    };

    void algorithm();
    void body();
    void checkpoint();

    void center_camera_on_body();

    std::unique_ptr<Agent> agent;
    b2World b2_world;
    BodySelectorWindow body_selector_window;
    WizardCheckpointSelectorWindow checkpoint_selector_window;
    double elapsed_time;
    torch::Tensor hidden_state;
    IO *io;
    double last_action_time;
    cpprl::Policy policy;
    ResourceManager *resource_manager;
    Random *rng;
    ScreenManager *screen_manager;
    State state;
    TrainingProgram program;
    glm::mat4 projection;

  public:
    TrainingWizardScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, Random &random, IO &io);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class TrainingWizardScreenFactory : public IScreenFactory
{
  private:
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    IO &io;
    Random &rng;

  public:
    TrainingWizardScreenFactory(ResourceManager &resource_manager, ScreenManager &screen_manager, IO &io, Random &rng)
        : resource_manager(resource_manager), screen_manager(screen_manager), io(io), rng(rng) {}

    inline std::shared_ptr<IScreen> make()
    {
        return std::make_shared<TrainingWizardScreen>(resource_manager, screen_manager, rng, io);
    }
};
}