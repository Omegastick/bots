#pragma once

class b2World;

#include "ui/training_wizard_screen/wizard_action.h"

namespace SingularityTrainer
{
class Agent;
class IO;
class Random;

class BodySelectorWindow
{
  private:
    IO *io;
    int last_selected_file;
    int selected_file;

  public:
    BodySelectorWindow(IO &io);

    WizardAction update(Agent &agent);
};
}