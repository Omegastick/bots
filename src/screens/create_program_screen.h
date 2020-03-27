#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "graphics/post_processing/post_proc_layer.h"
#include "environment/iecs_env.h"
#include "screens/iscreen.h"
#include "training/training_program.h"
#include "ui/create_program_screen/create_program_screen_state.h"
#include "ui/create_program_screen/create_program_screen_ui.h"

namespace ai
{
class AudioEngine;
class AlgorithmWindow;
class BodySelectorWindow;
class BrainWindow;
class Checkpointer;
class IO;
class Renderer;
class ResourceManager;
class RewardWindows;
class SaveLoadWindow;
class ScreenManager;
class Tabs;
class TrainScreenFactory;

class CreateProgramScreen : public IScreen
{
  private:
    void algorithm();
    void body();
    void brain();
    void rewards();
    void save_load();

    AudioEngine &audio_engine;
    std::unique_ptr<IEcsEnv> environment;
    IO &io;
    std::unique_ptr<TrainingProgram> program;
    glm::mat4 projection;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    CreateProgramScreenState state;
    TrainScreenFactory &train_screen_factory;
    std::unique_ptr<CreateProgramScreenUI> ui;

  public:
    CreateProgramScreen(std::unique_ptr<CreateProgramScreenUI> ui,
                        std::unique_ptr<IEcsEnv> environment,
                        std::unique_ptr<TrainingProgram> program,
                        AudioEngine &audio_engine,
                        IO &io,
                        ResourceManager &resource_manager,
                        ScreenManager &screen_manager,
                        TrainScreenFactory &train_screen_factory);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);

    void run_training();

    inline TrainingProgram &get_program() { return *program; }
};

class CreateProgramScreenFactory : public IScreenFactory
{
  private:
    AudioEngine &audio_engine;
    Checkpointer &checkpointer;
    CreateProgramScreenUIFactory &ui_factory;
    IO &io;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    TrainScreenFactory &train_screen_factory;

  public:
    CreateProgramScreenFactory(AudioEngine &audio_engine,
                               Checkpointer &checkpointer,
                               CreateProgramScreenUIFactory &ui_factory,
                               IO &io,
                               ResourceManager &resource_manager,
                               ScreenManager &screen_manager,
                               TrainScreenFactory &train_screen_factory)
        : audio_engine(audio_engine),
          checkpointer(checkpointer),
          ui_factory(ui_factory),
          io(io),
          resource_manager(resource_manager),
          screen_manager(screen_manager),
          train_screen_factory(train_screen_factory) {}

    std::shared_ptr<IScreen> make();
};
}