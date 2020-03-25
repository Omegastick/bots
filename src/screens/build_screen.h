#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Box2D/Box2D.h>
#include <entt/entity/entity.hpp>
#include <glm/mat4x4.hpp>

#include "environment/build_env.h"
#include "graphics/render_data.h"
#include "screens/iscreen.h"
#include "ui/build_screen/color_scheme_window.h"
#include "ui/build_screen/part_detail_window.h"
#include "ui/build_screen/part_selector_window.h"
#include "ui/build_screen/save_body_window.h"
#include "ui/build_screen/unlock_parts_window.h"

class b2World;

namespace ai
{
class Animator;
class CredentialsManager;
class IAudioEngine;
class IHttpClient;
class IO;
class IModuleFactory;
class ModuleTextureStore;
class Random;
class Renderer;
class ResourceManager;
class ScreenManager;

class BuildScreen : public IScreen
{
  private:
    IAudioEngine &audio_engine;
    BuildEnv build_env;
    std::unique_ptr<ColorSchemeWindow> color_scheme_window;
    float current_rotation;
    IModuleFactory &module_factory;
    ScreenManager &screen_manager;
    bool show_unlock_parts_window;
    IO &io;
    PartDetailWindow part_detail_window;
    std::unique_ptr<PartSelectorWindow> part_selector_window;
    b2World b2_world;
    std::unique_ptr<SaveBodyWindow> save_body_window;
    entt::entity module_to_place;
    std::string module_to_place_name;
    entt::entity selected_module;
    std::unique_ptr<UnlockPartsWindow> unlock_parts_window;
    glm::mat4 view;

  public:
    BuildScreen(BuildEnv &&build_env,
                std::unique_ptr<ColorSchemeWindow> color_scheme_window,
                std::unique_ptr<PartSelectorWindow> part_selector_window,
                std::unique_ptr<SaveBodyWindow> save_body_window,
                std::unique_ptr<UnlockPartsWindow> unlock_parts_window,
                IAudioEngine &audio_engine,
                IModuleFactory &module_factory,
                ResourceManager &resource_manager,
                ScreenManager &screen_manager,
                IO &io);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class BuildScreenFactory : public IScreenFactory
{
  private:
    Animator &animator;
    IAudioEngine &audio_engine;
    CredentialsManager &credentials_manager;
    IHttpClient &http_client;
    IModuleFactory &module_factory;
    ModuleTextureStore &module_texture_store;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    IO &io;

  public:
    BuildScreenFactory(Animator &animator,
                       IAudioEngine &audio_engine,
                       CredentialsManager &credentials_manager,
                       IHttpClient &http_client,
                       IModuleFactory &module_factory,
                       ModuleTextureStore &module_texture_store,
                       ResourceManager &resource_manager,
                       ScreenManager &screen_manager,
                       IO &io);

    std::shared_ptr<IScreen> make() override;
};
}