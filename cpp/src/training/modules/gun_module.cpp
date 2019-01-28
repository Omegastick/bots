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

    // Module links
    module_links.push_back(ModuleLink(0.5, 0.167, 90, this));
    module_links.push_back(ModuleLink(0, 0.5, 180, this));
    module_links.push_back(ModuleLink(-0.5, 0.167, 270, this));

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
        b2Transform global_transform = get_global_transform();
        b2Vec2 velocity = b2Mul(global_transform.q, b2Vec2(0, -100));
        b2Vec2 offset = b2Mul(global_transform.q, b2Vec2(0, -0.6));
        bullets.push_back(std::make_unique<Bullet>(global_transform.p + offset, velocity, *agent->rigid_body->body->GetWorld()));
    }
}

void GunModule::update()
{
    steps_since_last_shot++;
}
}