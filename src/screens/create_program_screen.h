#pragma once

#include <memory>

#include "graphics/post_proc_layer.h"
#include "screens/iscreen.h"
#include "training/training_program.h"
#include "ui/create_program_screen/create_program_screen_state.h"

namespace SingularityTrainer
{
class AgentFactory;
class AlgorithmWindow;
class BodySelectorWindow;
class RewardWindows;
class IEnvironment;
class IEnvironmentFactory;
class IO;
class Renderer;
class ResourceManager;
class ScreenManager;
class Tabs;

class CreateProgramScreen : public IScreen
{
  private:
    void algorithm();
    void body();
    void checkpoint();
    void rewards();
    void save_load();

    std::unique_ptr<AlgorithmWindow> algorithm_window;
    std::unique_ptr<BodySelectorWindow> body_selector_window;
    std::unique_ptr<RewardWindows> reward_windows;
    std::unique_ptr<IEnvironment> environment;
    IO &io;
    std::unique_ptr<TrainingProgram> program;
    PostProcLayer crt_post_proc_layer;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    CreateProgramScreenState state;
    std::unique_ptr<Tabs> tabs;

  public:
    CreateProgramScreen(std::unique_ptr<AlgorithmWindow> algorithm_window,
                        std::unique_ptr<BodySelectorWindow> body_selector_window,
                        std::unique_ptr<RewardWindows> reward_windows,
                        std::unique_ptr<IEnvironment> environment,
                        std::unique_ptr<TrainingProgram> program,
                        std::unique_ptr<Tabs> tabs,
                        IO &io,
                        ResourceManager &resource_manager,
                        ScreenManager &screen_manager);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class CreateProgramScreenFactory : public IScreenFactory
{
  private:
    AgentFactory &agent_factory;
    IEnvironmentFactory &env_factory;
    IO &io;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;

  public:
    CreateProgramScreenFactory(AgentFactory &agent_factory,
                               IEnvironmentFactory &env_factory,
                               IO &io,
                               ResourceManager &resource_manager,
                               ScreenManager &screen_manager)
        : agent_factory(agent_factory),
          env_factory(env_factory),
          io(io),
          resource_manager(resource_manager),
          screen_manager(screen_manager) {}

    std::shared_ptr<IScreen> make();
};
}