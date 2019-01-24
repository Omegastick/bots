#include <Box2D/Box2D.h>
#include <memory>
#include <vector>

#include "resource_manager.h"
#include "training/modules/base_module.h"
#include "training/modules/imodule.h"
#include "training/modules/module_link.h"

namespace SingularityTrainer
{
BaseModule::BaseModule(ResourceManager &resource_manager, b2Body &body)
{
    // Sprite
    // resource_manager.load_texture("base_module", "images/base_module.png");
    resource_manager.load_texture("base_module", "images/Arrow.png");
    sprite.setScale(0.1, 0.1);
    sprite.setTexture(*resource_manager.texture_store.get("base_module"));
    sprite.setOrigin(0.5, 0.5);

    // Box2D fixture
    b2PolygonShape shape;
    shape.SetAsBox(0.5, 0.5);
    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture = body.CreateFixture(&fixture_def);

    // Module links
    module_links.push_back(ModuleLink(0.5, 0, 90));
    module_links.push_back(ModuleLink(0, -0.5, 180));

    root = this;
}

BaseModule::~BaseModule() {}
}
