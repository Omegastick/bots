#include <vector>

#include "training/actions/shoot_action.h"
#include "training/modules/interfaces/ishootable.h"

namespace SingularityTrainer
{
ShootAction::ShootAction(IShootable &module) : module(module)
{
    flag_count = 1;
}
ShootAction::~ShootAction() {}

void ShootAction::act(std::vector<int> flags)
{
    if (flags[0] == 1)
    {
        module.shoot();
    }
}
}