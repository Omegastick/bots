#pragma once

#include <future>
#include <string>
#include <tuple>

#include "environment/test_env.h"
#include "screens/iscreen.h"
#include "third_party/di.hpp"
#include "training/training_program.h"

namespace ai
{
class CredentialsManager;
class IAudioEngine;
class IHttpClient;
class IO;
class Renderer;
class ScreenManager;

class MainMenuScreen : public IScreen
{
  public:
    struct UserInfo
    {
        long credits;
        long elo;
    };

  private:
    IAudioEngine &audio_engine;
    CredentialsManager &credentials_manager;
    IHttpClient &http_client;
    IO &io;
    IScreenFactory &build_screen_factory;
    IScreenFactory &create_program_screen_factory;
    IScreenFactory &multiplayer_screen_factory;
    TrainingProgram program;
    ScreenManager &screen_manager;
    TestEnv test_env;
    UserInfo user_info;
    std::future<UserInfo> user_info_future;
    bool user_info_received;
    std::string username;
    bool waiting_for_server;

  public:
    MainMenuScreen(IAudioEngine &audio_engine,
                   CredentialsManager &credentials_manager,
                   IHttpClient &http_client,
                   IO &io,
                   IScreenFactory &build_screen_factory,
                   IScreenFactory &create_program_screen_factory,
                   IScreenFactory &multiplayer_screen_factory,
                   ScreenManager &screen_manager);

    void draw(Renderer &renderer, bool lightweight = false);
    void on_show();
    void update(double delta_time);

    void build_body();
    std::future<UserInfo> get_user_info(const std::string &base_url, int timeout = 10);
    void multiplayer();
    void train_agent();
    void quit();
};

static auto BuildScreenFactoryType = [] {};
static auto CreateProgramScreenFactoryType = [] {};
static auto MultiplayerScreenFactoryType = [] {};

class MainMenuScreenFactory : public IScreenFactory
{
  private:
    IAudioEngine &audio_engine;
    CredentialsManager &credentials_manager;
    IHttpClient &http_client;
    IO &io;
    IScreenFactory &build_screen_factory;
    IScreenFactory &create_program_screen_factory;
    IScreenFactory &multiplayer_screen_factory;
    ScreenManager &screen_manager;

  public:
    BOOST_DI_INJECT(MainMenuScreenFactory,
                    IAudioEngine &audio_engine,
                    CredentialsManager &credentials_manager,
                    IHttpClient &http_client,
                    IO &io,
                    (named = BuildScreenFactoryType)
                        IScreenFactory &build_screen_factory,
                    (named = CreateProgramScreenFactoryType)
                        IScreenFactory &create_program_screen_factory,
                    (named = MultiplayerScreenFactoryType)
                        IScreenFactory &multiplayer_screen_factory,
                    ScreenManager &screen_manager)
        : audio_engine(audio_engine),
          credentials_manager(credentials_manager),
          http_client(http_client),
          io(io),
          build_screen_factory(build_screen_factory),
          create_program_screen_factory(create_program_screen_factory),
          multiplayer_screen_factory(multiplayer_screen_factory),
          screen_manager(screen_manager) {}

    virtual std::shared_ptr<IScreen> make()
    {
        return std::make_shared<MainMenuScreen>(audio_engine,
                                                credentials_manager,
                                                http_client,
                                                io,
                                                build_screen_factory,
                                                create_program_screen_factory,
                                                multiplayer_screen_factory,
                                                screen_manager);
    }
};
}