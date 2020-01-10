#include <iostream>
#include <mutex>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <spdlog/spdlog.h>

#include "bullet.h"
#include "audio/audio_engine.h"
#include "graphics/colors.h"
#include "graphics/renderers/renderer.h"
#include "training/bodies/body.h"
#include "training/effects/bullet_explosion.h"
#include "training/entities/bullet.h"
#include "training/environments/ienvironment.h"
#include "training/events/effect_triggered.h"
#include "training/events/entity_destroyed.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"
#include "training/training_program.h"

namespace ai
{
Bullet::Bullet(b2Vec2 position,
               b2Vec2 velocity,
               b2World &world,
               Body *owner,
               unsigned int id,
               IEnvironment &env,
               IAudioEngine &audio_engine)
    : IEntity(id, env),
      audio_engine(audio_engine),
      life(10),
      last_position(b2Vec2_zero),
      particle_color(cl_white),
      owner(owner),
      destroyed(false)
{
    sprite = std::make_unique<Sprite>();
    sprite->texture = "bullet";
    sprite->transform.set_scale(glm::vec2(0.2, 0.2));
    sprite->transform.set_position(glm::vec2(position.x, position.y));

    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, position, world, this, RigidBody::ParentTypes::Bullet);

    b2CircleShape rigid_body_shape;
    rigid_body_shape.m_radius = 0.1f;
    b2FixtureDef fixture_def;
    fixture_def.shape = &rigid_body_shape;
    fixture_def.density = 1;
    fixture_def.friction = 1;
    fixture_def.isSensor = false;
    rigid_body->body->CreateFixture(&fixture_def);
    rigid_body->body->SetBullet(true);

    rigid_body->body->ApplyForceToCenter(velocity, true);
}

void Bullet::draw(Renderer &renderer, bool /*lightweight*/)
{
    if (destroyed)
    {
        return;
    }

    b2Vec2 position = rigid_body->body->GetPosition();

    // Body
    sprite->transform.set_position(glm::vec2(position.x, position.y));
    renderer.draw(*sprite);

    // Trail
    if (last_position.x != b2Vec2_zero.x || last_position.y != b2Vec2_zero.y)
    {
        Line trail;
        trail.points.push_back({position.x, position.y});
        trail.widths.push_back(0.1f);
        trail.colors.push_back({1.0, 1.0, 1.0, 1.0});
        trail.points.push_back({last_position.x, last_position.y});
        trail.widths.push_back(0);
        trail.colors.push_back({1.0, 1.0, 1.0, 0.0});
        renderer.draw(trail);
    }

    last_position = position;
}

void Bullet::begin_contact(RigidBody *other)
{
    // Hill
    if (other->parent_type == RigidBody::ParentTypes::Hill)
    {
        return;
    }

    // Body
    if (other->parent_type == RigidBody::ParentTypes::Body && !destroyed)
    {
        const auto &reward_config = owner->get_environment()->get_reward_config();
        owner->get_environment()->change_reward(owner, reward_config.hit_enemy_reward);
        auto other_body = static_cast<Body *>(other->parent);
        other_body->get_environment()->change_reward(other_body, reward_config.hit_self_punishment);
        other_body->hit(1);

        auto &b2_transform = rigid_body->body->GetTransform();
        env.add_event(std::make_unique<EffectTriggered>(EffectTypes::BodyHit,
                                                        env.get_elapsed_time(),
                                                        Transform{b2_transform.p.x,
                                                                  b2_transform.p.y,
                                                                  b2_transform.q.GetAngle()}));
    }

    if (!destroyed)
    {
        destroyed = true;
        auto &b2_transform = rigid_body->body->GetTransform();
        env.add_event(std::make_unique<EffectTriggered>(EffectTypes::BulletExplosion,
                                                        env.get_elapsed_time(),
                                                        Transform{b2_transform.p.x,
                                                                  b2_transform.p.y,
                                                                  b2_transform.q.GetAngle()}));
        env.add_event(std::make_unique<EntityDestroyed>(id,
                                                        env.get_elapsed_time(),
                                                        Transform{b2_transform.p.x,
                                                                  b2_transform.p.y,
                                                                  b2_transform.q.GetAngle()}));
    }
}

void Bullet::end_contact(RigidBody * /*other*/) {}

bool Bullet::should_destroy()
{
    return destroyed;
}

void Bullet::update()
{
    --life;
    destroyed = destroyed || life <= 0;
}

BulletFactory::BulletFactory(IAudioEngine &audio_engine)
    : audio_engine(audio_engine) {}

std::unique_ptr<Bullet> BulletFactory::make(b2Vec2 position,
                                            b2Vec2 velocity,
                                            b2World &world,
                                            Body *owner,
                                            unsigned int id,
                                            IEnvironment &env)
{
    return std::make_unique<Bullet>(position,
                                    velocity,
                                    world,
                                    owner,
                                    id,
                                    env,
                                    audio_engine);
}
}