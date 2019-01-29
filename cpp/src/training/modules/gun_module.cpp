#include <Box2D/Box2D.h>
#include <iostream>
#include <memory>
#include <vector>

#include "resource_manager.h"
#include "training/actions/activate_action.h"
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
    b2PolygonShape body_shape;
    body_shape.SetAsBox(0.5, 0.333, b2Vec2(0, 0.167), 0);
    shapes.push_back(body_shape);
    b2PolygonShape barrel_shape;
    barrel_shape.SetAsBox(0.167, 0.333, b2Vec2(0, -0.167), 0);
    shapes.push_back(barrel_shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0.5, 0.167, 90, this));
    module_links.push_back(ModuleLink(0, 0.5, 180, this));
    module_links.push_back(ModuleLink(-0.5, 0.167, 270, this));

    actions.push_back(std::make_unique<ActivateAction>(this));

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

void GunModule::activate()
{
    if (steps_since_last_shot > cooldown)
    {
        steps_since_last_shot = 0;
        b2Transform global_transform = get_global_transform();
        b2Vec2 velocity = b2Mul(global_transform.q, b2Vec2(0, -100));
        b2Vec2 offset = b2Mul(global_transform.q, b2Vec2(0, -0.7));
        bullets.push_back(std::make_unique<Bullet>(global_transform.p + offset, velocity, *agent->rigid_body->body->GetWorld()));
    }
}

void GunModule::update()
{
    steps_since_last_shot++;
    
    for (const auto &bullet : bullets)
    {
        bullet->update();
    }

    for (int i = 0; i < bullets.size(); ++i)
    {
        if (bullets[i]->destroyed)
        {
            b2Body *body = bullets[i]->rigid_body->body;
            body->GetWorld()->DestroyBody(body);
            bullets.erase(bullets.begin() + i);
        }
    }
}
}