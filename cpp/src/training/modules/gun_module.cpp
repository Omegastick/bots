#include <memory>
#include <vector>

#include "training/actions/shoot_action.h"
#include "training/modules/gun_module.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
GunModule::GunModule() : cooldown(5), steps_since_last_shot(0)
{
    actions.push_back(std::make_unique<ShootAction>(*this));
}
GunModule::~GunModule() {}
}