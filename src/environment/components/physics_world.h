#pragma once

#include <memory>

class b2World;

namespace ai
{
struct PhysicsWorld
{
    std::unique_ptr<b2World> world = nullptr;
};
}