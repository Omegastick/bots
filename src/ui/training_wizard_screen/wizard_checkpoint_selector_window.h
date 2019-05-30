#pragma once

#include <memory>

#include <ui/training_wizard_screen/wizard_action.h>

namespace cpprl
{
class Policy;
}

namespace SingularityTrainer
{
class IO;

class WizardCheckpointSelectorWindow
{
  private:
    int last_selected_file;
    int selected_file;
    IO *io;

  public:
    WizardCheckpointSelectorWindow(IO &io);

    WizardAction update(cpprl::Policy &policy);
};
}