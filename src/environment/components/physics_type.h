#pragma once

namespace ai
{
struct PhysicsType
{
    enum Type
    {
        Bullet,
        Hill,
        Module,
        Wall
    };

    Type type;
};
}