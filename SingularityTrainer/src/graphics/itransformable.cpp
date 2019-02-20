#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "itransformable.h"

namespace SingularityTrainer
{
ITransformable::ITransformable()
    : origin(0, 0),
      position(0, 0),
      rotation(0),
      scale(1, 1),
      transform(),
      transform_needs_update(true) {}

void ITransformable::set_position(const glm::vec2 &position)
{
    this->position = position;
    transform_needs_update = true;
}

void ITransformable::set_rotation(float angle)
{
    rotation = static_cast<float>(fmod(angle, glm::radians(360.f)));
    if (rotation < 0)
        rotation += glm::radians(360.f);

    transform_needs_update = true;
}

void ITransformable::set_scale(const glm::vec2 &scale)
{
    this->scale = scale;
    transform_needs_update = true;
}

void ITransformable::set_origin(const glm::vec2 &origin)
{
    this->origin = origin;
    transform_needs_update = true;
}

void ITransformable::move(const glm::vec2 &offset)
{
    set_position(glm::vec2(position.x + offset.x, position.y + offset.y));
}

void ITransformable::rotate(float angle)
{
    set_rotation(rotation + angle);
}

void ITransformable::resize(const glm::vec2 &factor)
{
    set_scale(glm::vec2(scale.x * factor.x, scale.y * factor.y));
}

const glm::mat4 &ITransformable::get_transform() const
{
    if (transform_needs_update)
    {
        transform = glm::mat4(1.);
        transform = glm::translate(transform, glm::vec3(position.x, position.y, 0));
        transform = glm::rotate(transform, rotation, glm::vec3(0, 0, 1));
        transform = glm::translate(transform, glm::vec3(-origin.x, -origin.y, 0));
        transform = glm::scale(transform, glm::vec3(scale.x, scale.y, 1));

        transform_needs_update = false;
    }

    return transform;
}
}