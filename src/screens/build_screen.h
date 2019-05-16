#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/mat4x4.hpp>

#include "iscreen.h"
#include "ui/build_screen/part_selector_window.h"

namespace SingularityTrainer
{
class Renderer;
class ResourceManager;
class ScreenManager;
class Sprite;

class BuildScreen : public IScreen
{
  private:
    ResourceManager *resource_manager;
    ScreenManager *screen_manager;
    std::unique_ptr<PartSelectorWindow> part_selector_window;
    std::vector<std::string> available_parts;
    std::string selected_part;
    std::unique_ptr<Sprite> ghost;
    glm::mat4 projection;

  public:
    BuildScreen(ResourceManager &resource_manager, ScreenManager &screen_manager);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};
}