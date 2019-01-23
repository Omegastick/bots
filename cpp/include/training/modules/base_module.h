#pragma once

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "resource_manager.h"
#include "training/actions/iaction.h"
#include "training/modules/imodule.h"
#include "training/modules/interfaces/ishootable.h"

namespace SingularityTrainer
{
class BaseModule : public IModule
{
  public:
    BaseModule(ResourceManager &resource_manager, b2Body &body);
    ~BaseModule();
};
}