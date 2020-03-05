#pragma once

namespace ai
{
struct PhysicsType
{
    enum Type
    {
        Body,
        Bullet,
        Wall
    };

    Type type;
};
}