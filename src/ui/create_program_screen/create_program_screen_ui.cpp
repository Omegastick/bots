#include "create_program_screen_ui.h"
#include "misc/io.h"
#include "training/checkpointer.h"
#include "ui/create_program_screen/algorithm_window.h"
#include "ui/create_program_screen/body_selector_window.h"
#include "ui/create_program_screen/brain_window.h"
#include "ui/create_program_screen/reward_windows.h"
#include "ui/create_program_screen/save_load_window.h"
#include "ui/create_program_screen/tabs.h"

namespace ai
{
CreateProgramScreenUIFactory::CreateProgramScreenUIFactory(Checkpointer &checkpointer, IO &io)
    : checkpointer(checkpointer), io(io) {}

std::unique_ptr<CreateProgramScreenUI> CreateProgramScreenUIFactory::make()
{
    return std::unique_ptr<CreateProgramScreenUI>(
        new CreateProgramScreenUI{AlgorithmWindow(io),
                                  BodySelectorWindow(io),
                                  BrainWindow(checkpointer, io),
                                  RewardWindows(io),
                                  SaveLoadWindow(io),
                                  Tabs(io)});
}
}