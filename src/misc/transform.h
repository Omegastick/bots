#pragma once

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <msgpack.hpp>

namespace ai
{
class Transform
{
  private:
    glm::vec2 origin;
    glm::vec2 position;
    float rotation;
    glm::vec2 scale;
    int z;
    mutable glm::mat4 transform;
    mutable bool transform_needs_update;

  public:
    Transform();
    Transform(float x, float y, float rotation);

    glm::mat4 get() const;
    void move(glm::vec2 offset);
    void rotate(float angle);
    void resize(glm::vec2 scale);
    void set_origin(glm::vec2 origin);
    void set_position(glm::vec2 position);
    void set_rotation(float angle);
    void set_scale(glm::vec2 scale);
    void set_z(int z);

    inline glm::vec2 get_position() const { return position; }
    inline float get_rotation() const { return rotation; }
    inline glm::vec2 get_scale() const { return scale; }
    inline glm::vec2 get_origin() const { return origin; }
    inline int get_z() const { return z; }
};
}