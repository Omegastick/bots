#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class ModuleTextureStore;
struct Sprite;
class Renderer;
class ResourceManager;
class ScreenManager;

class TextureStoreTestScreen : public IScreen
{
  private:
    ResourceManager &resource_manager;
    std::vector<std::shared_ptr<IScreen>> &screens;
    std::vector<std::string> &screen_names;
    ScreenManager &screen_manager;
    glm::mat4 projection;
    ModuleTextureStore &module_texture_store;

  public:
    TextureStoreTestScreen(ModuleTextureStore &module_texture_store,
                           ScreenManager &screen_manager,
                           ResourceManager &resource_manager,
                           std::vector<std::shared_ptr<IScreen>> &screens,
                           std::vector<std::string> &screen_names);

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}