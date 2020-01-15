#pragma once

#include "ui/create_program_screen/algorithm_window.h"
#include "ui/create_program_screen/body_selector_window.h"
#include "ui/create_program_screen/brain_window.h"
#include "ui/create_program_screen/reward_windows.h"
#include "ui/create_program_screen/save_load_window.h"
#include "ui/create_program_screen/tabs.h"

namespace ai
{
class Checkpointer;
class IO;

struct CreateProgramScreenUI
{
    AlgorithmWindow algorithm_window;
    BodySelectorWindow body_selector_window;
    BrainWindow brain_window;
    RewardWindows reward_windows;
    SaveLoadWindow save_load_window;
    Tabs tabs;
};

class CreateProgramScreenUIFactory
{
  private:
    Checkpointer &checkpointer;
    IO &io;

  public:
    CreateProgramScreenUIFactory(Checkpointer &checkpointer, IO &io);

    std::unique_ptr<CreateProgramScreenUI> make();
};
}