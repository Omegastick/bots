#pragma once

#include <vector>
#include <memory>
#include <string>

#include "screens/iscreen.h"

namespace ai
{
class Renderer;
class ScreenManager;

class PlotTestScreen : public IScreen
{
  private:
    std::vector<std::shared_ptr<IScreen>> &screens;
    std::vector<std::string> &screen_names;
    ScreenManager &screen_manager;
    std::vector<double> xs, ys;

  public:
    PlotTestScreen(ScreenManager &screen_manager,
                   std::vector<std::shared_ptr<IScreen>> &screens,
                   std::vector<std::string> &screen_names);

    virtual void update(double delta_time);
    virtual void draw(Renderer &renderer, bool lightweight = false);
};
}