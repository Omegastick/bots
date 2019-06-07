#pragma once

#include "ui/create_program_screen/create_program_screen_state.h"

namespace SingularityTrainer
{
class IO;

class Tabs
{
  private:
    IO &io;
    CreateProgramScreenState selected_tab;

  public:
    Tabs(IO &io);

    CreateProgramScreenState update();
};
}