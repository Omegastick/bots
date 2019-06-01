#pragma once

#include "screens/iscreen.h"

namespace SingularityTrainer
{
class BuildScreenFactory;
class Renderer;
class ScreenManager;
class TrainingWizardScreenFactory;
class WatchScreenFactory;

class MainMenuScreen : public IScreen
{
  private:
    ScreenManager &screen_manager;
    BuildScreenFactory &build_screen_factory;
    TrainingWizardScreenFactory &training_wizard_screen_factory;
    WatchScreenFactory &watch_screen_factory;

  public:
    MainMenuScreen(ScreenManager &screen_manager,
                   BuildScreenFactory &build_screen_factory,
                   TrainingWizardScreenFactory &training_wizard_screen_factory,
                   WatchScreenFactory &watch_screen_factory);

    void draw(Renderer &renderer, bool lightweight = false);
    void update(double delta_time);
};

class MainMenuScreenFactory : public IScreenFactory
{
  private:
    ScreenManager &screen_manager;
    BuildScreenFactory &build_screen_factory;
    TrainingWizardScreenFactory &training_wizard_screen_factory;
    WatchScreenFactory &watch_screen_factory;

  public:
    MainMenuScreenFactory(ScreenManager &screen_manager,
                          TrainingWizardScreenFactory &training_wizard_screen_factory,
                          WatchScreenFactory &watch_screen_factory,
                          BuildScreenFactory &build_screen_factory)
        : screen_manager(screen_manager),
          build_screen_factory(build_screen_factory),
          training_wizard_screen_factory(training_wizard_screen_factory),
          watch_screen_factory(watch_screen_factory) {}

    virtual std::shared_ptr<IScreen> make()
    {
        return std::make_shared<MainMenuScreen>(screen_manager, build_screen_factory, training_wizard_screen_factory, watch_screen_factory);
    }
};
}