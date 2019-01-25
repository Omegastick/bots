#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "gui/colors.h"
#include "resource_manager.h"
#include "training/modules/base_module.h"
#include "training/modules/imodule.h"
#include "training/modules/module_link.h"
#include "training/agents/iagent.h"

namespace SingularityTrainer
{
BaseModule::BaseModule(ResourceManager &resource_manager, b2Body &body, IAgent *agent)
{
    // Sprite
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite.setTexture(*resource_manager.texture_store.get("base_module"));
    sprite.setScale(0.01, 0.01);
    sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
    sprite.setColor(cl_white);

    // Box2D
    shape.SetAsBox(0.5, 0.5);
    transform.SetIdentity();
    rotation_rad = 0;

    // Module links
    module_links.push_back(ModuleLink(0.5, 0, 90, this));
    module_links.push_back(ModuleLink(0, -0.5, 180, this));

    root = this;
    this->agent = agent;
}

BaseModule::~BaseModule() {}
}
