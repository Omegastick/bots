#pragma once

#include "screens/iscreen.h"
#include "training/training_program.h"

namespace SingularityTrainer
{
class BuildScreenFactory;
class CreateProgramScreenFactory;
class MultiplayerScreenFactory;
class Renderer;
class ScreenManager;

class MainMenuScreen : public IScreen
{
  private:
    BuildScreenFactory &build_screen_factory;
    CreateProgramScreenFactory &create_program_screen_factory;
    MultiplayerScreenFactory &multiplayer_screen_factory;
    TrainingProgram program;
    ScreenManager &screen_manager;

  public:
    MainMenuScreen(BuildScreenFactory &build_screen_factory,
                   CreateProgramScreenFactory &create_program_screen_factory,
                   MultiplayerScreenFactory &multiplayer_screen_factory,
                   ScreenManager &screen_manager);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class MainMenuScreenFactory : public IScreenFactory
{
  private:
    BuildScreenFactory &build_screen_factory;
    CreateProgramScreenFactory &create_program_screen_factory;
    MultiplayerScreenFactory &multiplayer_screen_factory;
    ScreenManager &screen_manager;

  public:
    MainMenuScreenFactory(BuildScreenFactory &build_screen_factory,
                          CreateProgramScreenFactory &create_program_screen_factory,
                          MultiplayerScreenFactory &multiplayer_screen_factory,
                          ScreenManager &screen_manager)
        : build_screen_factory(build_screen_factory),
          create_program_screen_factory(create_program_screen_factory),
          multiplayer_screen_factory(multiplayer_screen_factory),
          screen_manager(screen_manager) {}

    virtual std::shared_ptr<IScreen> make()
    {
        return std::make_shared<MainMenuScreen>(build_screen_factory,
                                                create_program_screen_factory,
                                                multiplayer_screen_factory,
                                                screen_manager);
    }
};
}