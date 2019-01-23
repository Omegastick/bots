#include <Box2D/Box2D.h>
#include <memory>
#include <vector>

#include "resource_manager.h"
#include "training/actions/shoot_action.h"
#include "training/modules/gun_module.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
GunModule::GunModule(ResourceManager &resource_manager, b2Body &body) : cooldown(5), steps_since_last_shot(0)
{
    actions.push_back(std::make_unique<ShootAction>(*this));

    resource_manager.load_texture("gun_module", "images/modules/gun_module.png");

    sprite.setScale(0.01, 0.01);
    sprite.setTexture(*resource_manager.texture_store.get("gun_module"));
    sprite.setOrigin(0.5, 0.5);

    b2PolygonShape shape;
    shape.SetAsBox(0.5, 0.5);
    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture = body.CreateFixture(&fixture_def);
}

void GunModule::shoot() {}
}