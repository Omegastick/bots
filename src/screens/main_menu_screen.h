#pragma once

#include "screens/iscreen.h"
#include "third_party/di.hpp"
#include "training/training_program.h"

namespace SingularityTrainer
{
class Renderer;
class ScreenManager;

class MainMenuScreen : public IScreen
{
  private:
    IScreenFactory &build_screen_factory;
    IScreenFactory &create_program_screen_factory;
    IScreenFactory &multiplayer_screen_factory;
    TrainingProgram program;
    ScreenManager &screen_manager;

  public:
    MainMenuScreen(IScreenFactory &build_screen_factory,
                   IScreenFactory &create_program_screen_factory,
                   IScreenFactory &multiplayer_screen_factory,
                   ScreenManager &screen_manager);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);

    void train_agent();
    void build_body();
    void multiplayer();
    void quit();
};

static auto BuildScreenFactoryType = [] {};
static auto CreateProgramScreenFactoryType = [] {};
static auto MultiplayerScreenFactoryType = [] {};

class MainMenuScreenFactory : public IScreenFactory
{
  private:
    IScreenFactory &build_screen_factory;
    IScreenFactory &create_program_screen_factory;
    IScreenFactory &multiplayer_screen_factory;
    ScreenManager &screen_manager;

  public:
    BOOST_DI_INJECT(MainMenuScreenFactory,
                    (named = BuildScreenFactoryType)
                        IScreenFactory &build_screen_factory,
                    (named = CreateProgramScreenFactoryType)
                        IScreenFactory &create_program_screen_factory,
                    (named = MultiplayerScreenFactoryType)
                        IScreenFactory &multiplayer_screen_factory,
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