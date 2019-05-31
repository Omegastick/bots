#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Box2D/Box2D.h>
#include <glm/mat4x4.hpp>

#include "graphics/sprite.h"
#include "screens/iscreen.h"
#include "training/modules/gun_module.h"
#include "ui/build_screen/part_detail_window.h"
#include "ui/build_screen/part_selector_window.h"
#include "ui/build_screen/save_body_window.h"
#include "ui/build_screen/body_builder.h"

class b2World;

namespace SingularityTrainer
{
class IO;
class Random;
class Renderer;
class ResourceManager;
class ScreenManager;

class BuildScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    ScreenManager *screen_manager;
    IO *io;
    PartDetailWindow part_detail_window;
    PartSelectorWindow part_selector_window;
    std::vector<std::string> available_parts;
    glm::mat4 projection;
    b2World b2_world;
    SaveBodyWindow save_body_window;
    BodyBuilder body_builder;
    std::shared_ptr<IModule> module_to_place;
    std::shared_ptr<IModule> selected_module;
    Sprite test_sprite;
    int current_rotation;

  public:
    BuildScreen(ResourceManager &resource_manager, ScreenManager &screen_manager, IO &io, Random &rng);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class BuildScreenFactory : public IScreenFactory
{
  private:
    ResourceManager &resource_manager;
    ScreenManager &screen_manager;
    IO &io;
    Random &rng;

  public:
    BuildScreenFactory(ResourceManager &resource_manager, ScreenManager &screen_manager, IO &io, Random &rng)
        : resource_manager(resource_manager), screen_manager(screen_manager), io(io), rng(rng) {}

    inline std::shared_ptr<IScreen> make()
    {
        return std::make_shared<BuildScreen>(resource_manager, screen_manager, io, rng);
    }
};
}