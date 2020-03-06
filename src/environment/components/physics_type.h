#pragma once

namespace ai
{
struct PhysicsType
{
    enum Type
    {
        Body,
        Bullet,
        Hill,
        Wall
    };

    Type type;
};
}