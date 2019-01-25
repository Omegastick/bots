#include <Box2D/Box2D.h>
#include <iostream>
#include <memory>
#include <vector>

#include "resource_manager.h"
#include "training/actions/shoot_action.h"
#include "training/agents/iagent.h"
#include "training/entities/bullet.h"
#include "training/modules/gun_module.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
GunModule::GunModule(ResourceManager &resource_manager, b2Body &body, IAgent *agent) : cooldown(10), steps_since_last_shot(0)
{
    // Sprite
    resource_manager.load_texture("gun_module", "images/gun_module.png");
    sprite.setScale(0.01, 0.01);
    sprite.setTexture(*resource_manager.texture_store.get("gun_module"));
    sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

    // Box2D fixture
    shape.SetAsBox(0.5, 0.5);
    transform.SetIdentity();
    rotation_rad = 0;

    // Module links
    module_links.push_back(ModuleLink(-0.5, -0.167, -90, this));
    module_links.push_back(ModuleLink(0.5, -0.167, 90, this));
    module_links.push_back(ModuleLink(0, -0.5, 180, this));

    actions.push_back(std::make_unique<ShootAction>(this));

    this->agent = agent;
}

GunModule::~GunModule() {}

void GunModule::draw(sf::RenderTarget &render_target)
{
    IModule::draw(render_target);
    for (const auto &bullet : bullets)
    {
        bullet->draw(render_target);
    }
}

void GunModule::shoot()
{
    if (steps_since_last_shot > cooldown)
    {
        steps_since_last_shot = 0;
        b2Vec2 offset(0, -0.6);
        b2Rot angle = b2Rot(agent->rigid_body->body->GetAngle());
        b2Vec2 position = get_global_position() + b2Mul(angle, offset);
        b2Vec2 velocity = b2Mul(angle, b2Vec2(0, -100));
        bullets.push_back(std::make_unique<Bullet>(position, velocity, *agent->rigid_body->body->GetWorld()));
    }
}

void GunModule::update()
{
    steps_since_last_shot++;
}
}