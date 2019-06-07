#pragma once

#include <memory>

#include "graphics/post_proc_layer.h"
#include "screens/iscreen.h"
#include "training/training_program.h"
#include "ui/create_program_screen/create_program_screen_state.h"
#include "ui/create_program_screen/tabs.h"

namespace SingularityTrainer
{
class IO;
class Renderer;
class ResourceManager;
class ScreenManager;

class CreateProgramScreen : public IScreen
{
  private:
    void algorithm();
    void body();
    void checkpoint();
    void rewards();
    void save_load();

    IO &io;
    std::unique_ptr<TrainingProgram> program;
    PostProcLayer crt_post_proc_layer;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    CreateProgramScreenState state;
    std::unique_ptr<Tabs> tabs;

  public:
    CreateProgramScreen(std::unique_ptr<TrainingProgram> program,
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
    IO &io;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;

  public:
    CreateProgramScreenFactory(IO &io,
                               ResourceManager &resource_manager,
                               ScreenManager &screen_manager)
        : io(io),
          resource_manager(resource_manager),
          screen_manager(screen_manager) {}

    inline std::shared_ptr<IScreen> make()
    {
        return std::make_shared<CreateProgramScreen>(std::make_unique<TrainingProgram>(),
                                                     std::make_unique<Tabs>(io),
                                                     io,
                                                     resource_manager,
                                                     screen_manager);
    }
};
}