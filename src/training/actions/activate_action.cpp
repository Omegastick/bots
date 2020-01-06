#include <vector>

#include "training/actions/activate_action.h"
#include "training/modules/interfaces/iactivatable.h"

namespace ai
{
ActivateAction::ActivateAction(IActivatable *module) : module(module)
{
    flag_count = 1;
}
ActivateAction::~ActivateAction() {}

void ActivateAction::act(std::vector<int> flags)
{
    if (flags[0] == 1)
    {
        module->activate();
    }
}
}