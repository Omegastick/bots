#pragma once

class b2Body;

namespace ai
{
struct PhysicsBody
{
    b2Body *body = nullptr;
};
}