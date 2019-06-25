#pragma once

#include <memory>

#include <glm/mat4x4.hpp>

#include "graphics/post_proc_layer.h"
#include "screens/iscreen.h"
#include "training/training_program.h"
#include "ui/create_program_screen/create_program_screen_state.h"

namespace SingularityTrainer
{
class BodyFactory;
class AlgorithmWindow;
class BodySelectorWindow;
class BrainWindow;
class Checkpointer;
class IEnvironment;
class IEnvironmentFactory;
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

    std::unique_ptr<AlgorithmWindow> algorithm_window;
    std::unique_ptr<BodySelectorWindow> body_selector_window;
    std::unique_ptr<BrainWindow> brain_window;
    std::unique_ptr<IEnvironment> environment;
    IO &io;
    std::unique_ptr<TrainingProgram> program;
    glm::mat4 projection;
    PostProcLayer crt_post_proc_layer;
    ResourceManager &resource_manager;
    std::unique_ptr<RewardWindows> reward_windows;
    std::unique_ptr<SaveLoadWindow> save_load_window;
    ScreenManager &screen_manager;
    CreateProgramScreenState state;
    std::unique_ptr<Tabs> tabs;
    TrainScreenFactory &train_screen_factory;

  public:
    CreateProgramScreen(std::unique_ptr<AlgorithmWindow> algorithm_window,
                        std::unique_ptr<BodySelectorWindow> body_selector_window,
                        std::unique_ptr<BrainWindow> brain_window,
                        std::unique_ptr<RewardWindows> reward_windows,
                        std::unique_ptr<IEnvironment> environment,
                        std::unique_ptr<TrainingProgram> program,
                        std::unique_ptr<SaveLoadWindow> save_load_window,
                        std::unique_ptr<Tabs> tabs,
                        IO &io,
                        ResourceManager &resource_manager,
                        ScreenManager &screen_manager,
                        TrainScreenFactory &train_screen_factory);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class CreateProgramScreenFactory : public IScreenFactory
{
  private:
    BodyFactory &body_factory;
    Checkpointer &checkpointer;
    IEnvironmentFactory &env_factory;
    IO &io;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    TrainScreenFactory &train_screen_factory;

  public:
    CreateProgramScreenFactory(BodyFactory &body_factory,
                               Checkpointer &checkpointer,
                               IEnvironmentFactory &env_factory,
                               IO &io,
                               ResourceManager &resource_manager,
                               ScreenManager &screen_manager,
                               TrainScreenFactory &train_screen_factory)
        : body_factory(body_factory),
        checkpointer(checkpointer),
          env_factory(env_factory),
          io(io),
          resource_manager(resource_manager),
          screen_manager(screen_manager),
          train_screen_factory(train_screen_factory) {}

    std::shared_ptr<IScreen> make();
};
}