#pragma once

#include <glm/glm.hpp>

namespace SingularityTrainer
{
class ITransformable
{
  private:
    glm::vec2 origin;
    glm::vec2 position;
    float rotation;
    glm::vec2 scale;
    mutable glm::mat4 transform;
    mutable bool transform_needs_update;

  public:
    ITransformable();
    virtual ~ITransformable() = 0;

    virtual void set_position(const glm::vec2 &position);
    virtual void set_rotation(float angle);
    virtual void set_scale(const glm::vec2 &scale);
    virtual void set_origin(const glm::vec2 &origin);
    virtual inline glm::vec2 get_position() const { return position; };
    virtual inline float get_rotation() const { return rotation; };
    virtual inline glm::vec2 get_scale() const { return scale; };
    virtual inline glm::vec2 get_origin() const { return origin; };
    virtual void move(const glm::vec2 &offset);
    virtual void rotate(float angle);
    virtual void resize(const glm::vec2 &scale);
    virtual const glm::mat4 &get_transform() const;
};

inline ITransformable::~ITransformable() {}
}