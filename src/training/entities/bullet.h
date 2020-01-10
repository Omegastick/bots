#pragma once

#include <memory>
#include <mutex>

#include <Box2D/Box2D.h>
#include <glm/vec4.hpp>
#include <trompeloeil.hpp>

#include "training/entities/ientity.h"
#include "training/environments/ienvironment.h"
#include "training/icollidable.h"

namespace ai
{
class IAudioEngine;
class Body;
class RigidBody;
struct Sprite;

class Bullet : public ICollidable, public IEntity
{
  private:
    IAudioEngine &audio_engine;
    int life;
    b2Vec2 last_position;
    std::unique_ptr<Sprite> sprite;
    glm::vec4 particle_color;
    Body *owner;
    bool destroyed;

  public:
    Bullet(b2Vec2 position,
           b2Vec2 velocity,
           b2World &world,
           Body *owner,
           unsigned int id,
           IEnvironment &env,
           IAudioEngine &audio_engine);

    void begin_contact(RigidBody *other) override;
    void end_contact(RigidBody *other) override;
    void draw(Renderer &renderer, bool lightweight = false) override;
    bool should_destroy() override;
    void update() override;
};

class IBulletFactory
{
  public:
    virtual std::unique_ptr<Bullet> make(b2Vec2 position,
                                         b2Vec2 velocity,
                                         b2World &world,
                                         Body *owner,
                                         unsigned int id,
                                         IEnvironment &env) = 0;
};

class BulletFactory : public IBulletFactory
{
  private:
    IAudioEngine &audio_engine;

  public:
    explicit BulletFactory(IAudioEngine &audio_engine);

    std::unique_ptr<Bullet> make(b2Vec2 position,
                                 b2Vec2 velocity,
                                 b2World &world,
                                 Body *owner,
                                 unsigned int id,
                                 IEnvironment &env);
};

class MockBulletFactory : public trompeloeil::mock_interface<IBulletFactory>
{
  public:
    IMPLEMENT_MOCK6(make);
};
}