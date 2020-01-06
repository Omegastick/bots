#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Box2D/Box2D.h>

#include "graphics/render_data.h"
#include "screens/iscreen.h"
#include "training/modules/gun_module.h"
#include "ui/build_screen/color_scheme_window.h"
#include "ui/build_screen/part_detail_window.h"
#include "ui/build_screen/part_selector_window.h"
#include "ui/build_screen/save_body_window.h"
#include "ui/build_screen/body_builder.h"
#include "ui/build_screen/unlock_parts_window.h"

class b2World;

namespace ai
{
class Animator;
class CredentialsManager;
class IHttpClient;
class IO;
class ModuleFactory;
class ModuleTextureStore;
class Random;
class Renderer;
class ResourceManager;
class ScreenManager;

class BuildScreen : public IScreen
{
  private:
    std::unique_ptr<ColorSchemeWindow> color_scheme_window;
    float current_rotation;
    ModuleFactory &module_factory;
    ScreenManager &screen_manager;
    std::string selected_module_name;
    bool show_unlock_parts_window;
    IO &io;
    PartDetailWindow part_detail_window;
    std::unique_ptr<PartSelectorWindow> part_selector_window;
    b2World b2_world;
    std::unique_ptr<SaveBodyWindow> save_body_window;
    BodyBuilder body_builder;
    std::shared_ptr<IModule> module_to_place;
    std::shared_ptr<IModule> selected_module;
    std::unique_ptr<UnlockPartsWindow> unlock_parts_window;

  public:
    BuildScreen(BodyBuilder &&body_builder,
                std::unique_ptr<ColorSchemeWindow> color_scheme_window,
                std::unique_ptr<PartSelectorWindow> part_selector_window,
                std::unique_ptr<SaveBodyWindow> save_body_window,
                std::unique_ptr<UnlockPartsWindow> unlock_parts_window,
                ModuleFactory &module_factory,
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
    BodyBuilderFactory &body_builder_factory;
    CredentialsManager &credentials_manager;
    IHttpClient &http_client;
    ModuleFactory &module_factory;
    ModuleTextureStore &module_texture_store;
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    IO &io;

  public:
    BuildScreenFactory(Animator &animator,
                       BodyBuilderFactory &body_builder_factory,
                       CredentialsManager &credentials_manager,
                       IHttpClient &http_client,
                       ModuleFactory &module_factory,
                       ModuleTextureStore &module_texture_store,
                       ResourceManager &resource_manager,
                       ScreenManager &screen_manager,
                       IO &io)
        : animator(animator),
          body_builder_factory(body_builder_factory),
          credentials_manager(credentials_manager),
          http_client(http_client),
          module_factory(module_factory),
          module_texture_store(module_texture_store),
          resource_manager(resource_manager),
          screen_manager(screen_manager),
          io(io) {}

    inline std::shared_ptr<IScreen> make()
    {
        return std::make_shared<BuildScreen>(std::move(*body_builder_factory.make()),
                                             std::make_unique<ColorSchemeWindow>(io),
                                             std::make_unique<PartSelectorWindow>(
                                                 credentials_manager,
                                                 http_client,
                                                 io,
                                                 module_texture_store,
                                                 resource_manager),
                                             std::make_unique<SaveBodyWindow>(animator, io),
                                             std::make_unique<UnlockPartsWindow>(
                                                 credentials_manager,
                                                 http_client,
                                                 io,
                                                 module_texture_store,
                                                 resource_manager),
                                             module_factory,
                                             resource_manager,
                                             screen_manager,
                                             io);
    }
};
}