#include <Box2D/Box2D.h>
#include <iostream>
#include <memory>
#include <vector>

#include "graphics/sprite.h"
#include "resource_manager.h"
#include "training/actions/activate_action.h"
#include "training/agents/iagent.h"
#include "training/entities/bullet.h"
#include "training/modules/gun_module.h"
#include "training/modules/imodule.h"

namespace SingularityTrainer
{
GunModule::GunModule(ResourceManager &resource_manager, b2Body &body, IAgent *agent) : cooldown(3), steps_since_last_shot(0)
{
    // Sprite
    sprite = std::make_unique<Sprite>("gun_module");
    sprite->set_scale(glm::vec2(1, 1));
    sprite->set_origin(sprite->get_center());

    // Box2D fixture
    b2PolygonShape body_shape;
    body_shape.SetAsBox(0.5, 0.333, b2Vec2(0, 0.167), 0);
    shapes.push_back(body_shape);
    b2PolygonShape barrel_shape;
    barrel_shape.SetAsBox(0.167, 0.333, b2Vec2(0, -0.167), 0);
    shapes.push_back(barrel_shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0.5, -0.167, 90, this));
    module_links.push_back(ModuleLink(0, -0.5, 180, this));
    module_links.push_back(ModuleLink(-0.5, -0.167, 270, this));

    actions.push_back(std::make_unique<ActivateAction>(this));

    this->agent = agent;
}

GunModule::~GunModule() {}

RenderData GunModule::get_render_data(bool lightweight)
{
    return IModule::get_render_data(lightweight);
    // for (const auto &bullet : bullets)
    // {
    //     bullet->draw(render_target, lightweight);
    // }
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