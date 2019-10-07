#pragma once

class b2World;

#include "ui/training_wizard_screen/wizard_action.h"

namespace SingularityTrainer
{
class Body;
class IO;
class Random;
struct TrainingProgram;

class BodySelectorWindow
{
  private:
    IO *io;
    int last_selected_file;
    int selected_file;

  public:
    BodySelectorWindow(IO &io);

    WizardAction update(Body &body, TrainingProgram &program);
};
}