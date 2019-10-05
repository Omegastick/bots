#pragma once

#include <string>

#include "screens/iscreen.h"
#include "third_party/di.hpp"
#include "training/training_program.h"

namespace SingularityTrainer
{
class CredentialsManager;
class IO;
class Renderer;
class ScreenManager;

class MainMenuScreen : public IScreen
{
  private:
    CredentialsManager &credentials_manager;
    IO &io;
    IScreenFactory &build_screen_factory;
    IScreenFactory &create_program_screen_factory;
    IScreenFactory &multiplayer_screen_factory;
    TrainingProgram program;
    ScreenManager &screen_manager;
    std::string username;

  public:
    MainMenuScreen(CredentialsManager &credentials_manager,
                   IO &io,
                   IScreenFactory &build_screen_factory,
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
    CredentialsManager &credentials_manager;
    IO &io;
    IScreenFactory &build_screen_factory;
    IScreenFactory &create_program_screen_factory;
    IScreenFactory &multiplayer_screen_factory;
    ScreenManager &screen_manager;

  public:
    BOOST_DI_INJECT(MainMenuScreenFactory,
                    CredentialsManager &credentials_manager,
                    IO &io,
                    (named = BuildScreenFactoryType)
                        IScreenFactory &build_screen_factory,
                    (named = CreateProgramScreenFactoryType)
                        IScreenFactory &create_program_screen_factory,
                    (named = MultiplayerScreenFactoryType)
                        IScreenFactory &multiplayer_screen_factory,
                    ScreenManager &screen_manager)
        : credentials_manager(credentials_manager),
          io(io),
          build_screen_factory(build_screen_factory),
          create_program_screen_factory(create_program_screen_factory),
          multiplayer_screen_factory(multiplayer_screen_factory),
          screen_manager(screen_manager) {}

    virtual std::shared_ptr<IScreen> make()
    {
        return std::make_shared<MainMenuScreen>(credentials_manager,
                                                io,
                                                build_screen_factory,
                                                create_program_screen_factory,
                                                multiplayer_screen_factory,
                                                screen_manager);
    }
};
}