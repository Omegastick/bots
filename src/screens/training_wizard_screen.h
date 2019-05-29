#pragma once

#include <Box2D/Box2D.h>

#include <memory>

#include "screens/iscreen.h"
#include "training/training_program.h"
#include "ui/shared/body_selector_window.h"

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

    std::unique_ptr<Agent> agent;
    b2World b2_world;
    BodySelectorWindow body_selector_window;
    IO *io;
    ResourceManager *resource_manager;
    Random *rng;
    ScreenManager *screen_manager;
    State state;
    TrainingProgram program;

  public:
    TrainingWizardScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, Random &random, IO &io);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};
}