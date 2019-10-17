#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include <Box2D/Box2D.h>

#include "training/icollidable.h"

namespace SingularityTrainer
{
class Body;
class Renderer;
struct RewardConfig;
class RigidBody;
class Sprite;

class Hill : public ICollidable
{
  private:
    std::function<void(const std::unordered_map<Body *, int> &)> callback;
    std::unique_ptr<Sprite> sprite;
    std::unordered_map<Body *, int> occupants;

  public:
    Hill(float x, float y, b2World &world);

    void draw(Renderer &renderer, bool lightweight = false);
    void begin_contact(RigidBody *other);
    void end_contact(RigidBody *other);
    void update();
    void register_callback(std::function<void(const std::unordered_map<Body *, int> &)> callback);
    void reset();

    std::unique_ptr<RigidBody> rigid_body;
};
}