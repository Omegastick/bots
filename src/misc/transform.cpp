#include <Box2D/Common/b2Math.h>
#include <doctest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <spdlog/spdlog.h>

#include "transform.h"

namespace glm
{
std::ostream &operator<<(std::ostream &os, const vec2 &value)
{
    os << "{" << value.x << ", " << value.y << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, const vec3 &value)
{
    os << "{" << value.x << ", " << value.y << ", " << value.z << "}";
    return os;
}
}

namespace ai
{
Transform::Transform()
    : origin(0, 0),
      position(0, 0),
      rotation(0),
      scale(1, 1),
      z(0),
      transform(),
      transform_needs_update(true) {}

Transform::Transform(float x, float y, float rot) : Transform()
{
    set_position({x, y});
    set_rotation(rot);
}

Transform::Transform(const b2Transform &b2_transform)
    : origin(0, 0),
      position(b2_transform.p.x, b2_transform.p.y),
      rotation(b2_transform.q.GetAngle()),
      scale(1, 1),
      z(0),
      transform(),
      transform_needs_update(true) {}

Transform::operator b2Transform()
{
    return b2Transform(b2Vec2{position.x, position.y}, b2Rot(rotation));
}

void Transform::set_origin(glm::vec2 origin)
{
    transform_needs_update = true;
    this->origin = origin;
}

void Transform::set_position(glm::vec2 position)
{
    transform_needs_update = true;
    this->position = position;
}

void Transform::set_rotation(float angle)
{
    transform_needs_update = true;
    rotation = angle;
}

void Transform::set_scale(glm::vec2 scale)
{
    transform_needs_update = true;
    this->scale = scale;
}

void Transform::set_z(int z)
{
    this->z = z;
}

void Transform::move(glm::vec2 offset)
{
    transform_needs_update = true;
    position += offset;
}

void Transform::rotate(float angle)
{
    transform_needs_update = true;
    rotation += angle;
}

void Transform::resize(glm::vec2 factor)
{
    transform_needs_update = true;
    scale *= factor;
}

glm::mat4 Transform::get() const
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

bool approx_equal(glm::vec3 a, glm::vec3 b)
{
    return a.x == doctest::Approx(b.x) &&
           a.y == doctest::Approx(b.y) &&
           a.z == doctest::Approx(b.z);
}

TEST_CASE("Transform")
{
    Transform transform;

    SUBCASE("move()")
    {
        SUBCASE("Changes the translation of the transform")
        {
            transform.move({2, 3});
            DOCTEST_CHECK(approx_equal(glm::vec3(transform.get()[3]), glm::vec3{2, 3, 0}));
        }

        SUBCASE("Multiple calls stack the translations")
        {
            transform.move({1, 2});
            DOCTEST_CHECK(approx_equal(glm::vec3(transform.get()[3]), glm::vec3{1, 2, 0}));
            transform.move({-3, -4});
            DOCTEST_CHECK(approx_equal(glm::vec3(transform.get()[3]), glm::vec3{-2, -2, 0}));
        }
    }

    SUBCASE("rotate()")
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;

        SUBCASE("Changes the rotation of the transform")
        {
            transform.rotate(1);
            glm::decompose(transform.get(), scale, rotation, translation, skew, perspective);
            DOCTEST_CHECK(approx_equal(glm::eulerAngles(rotation), glm::vec3{0, 0, 1}));
        }

        SUBCASE("Multiple calls stack")
        {
            transform.rotate(-1);
            glm::decompose(transform.get(), scale, rotation, translation, skew, perspective);
            DOCTEST_CHECK(approx_equal(glm::eulerAngles(rotation), glm::vec3{0, 0, -1}));
            transform.rotate(2);
            glm::decompose(transform.get(), scale, rotation, translation, skew, perspective);
            DOCTEST_CHECK(approx_equal(glm::eulerAngles(rotation), glm::vec3{0, 0, 1}));
        }
    }

    SUBCASE("resize()")
    {

        SUBCASE("Changes the scale of the transform")
        {
            transform.resize({2, 3});
            auto t = transform.get();
            DOCTEST_CHECK(approx_equal(glm::vec3(t[0][0], t[1][1], t[2][2]), glm::vec3{2, 3, 1}));
        }

        SUBCASE("Multiple calls stack")
        {
            transform.resize({2, 3});
            auto t = transform.get();
            DOCTEST_CHECK(approx_equal(glm::vec3(t[0][0], t[1][1], t[2][2]), glm::vec3{2, 3, 1}));
            transform.resize({-1, 2});
            t = transform.get();
            DOCTEST_CHECK(approx_equal(glm::vec3(t[0][0], t[1][1], t[2][2]), glm::vec3{-2, 6, 1}));
        }
    }

    SUBCASE("set_position()")
    {
        SUBCASE("Changes the translation of the transform")
        {
            transform.set_position({2, 3});
            DOCTEST_CHECK(approx_equal(glm::vec3(transform.get()[3]), glm::vec3{2, 3, 0}));
        }

        SUBCASE("Multiple calls don't stack the translations")
        {
            transform.set_position({1, 2});
            DOCTEST_CHECK(approx_equal(glm::vec3(transform.get()[3]), glm::vec3{1, 2, 0}));
            transform.set_position({-3, -4});
            DOCTEST_CHECK(approx_equal(glm::vec3(transform.get()[3]), glm::vec3{-3, -4, 0}));
        }
    }

    SUBCASE("set_rotation()")
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;

        SUBCASE("Changes the rotation of the transform")
        {
            transform.set_rotation(1);
            glm::decompose(transform.get(), scale, rotation, translation, skew, perspective);
            DOCTEST_CHECK(approx_equal(glm::eulerAngles(rotation), glm::vec3{0, 0, 1}));
        }

        SUBCASE("Multiple calls don't stack")
        {
            transform.set_rotation(-1);
            glm::decompose(transform.get(), scale, rotation, translation, skew, perspective);
            DOCTEST_CHECK(approx_equal(glm::eulerAngles(rotation), glm::vec3{0, 0, -1}));
            transform.set_rotation(2);
            glm::decompose(transform.get(), scale, rotation, translation, skew, perspective);
            DOCTEST_CHECK(approx_equal(glm::eulerAngles(rotation), glm::vec3{0, 0, 2}));
        }
    }

    SUBCASE("set_scale()")
    {

        SUBCASE("Changes the scale of the transform")
        {
            transform.set_scale({2, 3});
            auto t = transform.get();
            DOCTEST_CHECK(approx_equal(glm::vec3(t[0][0], t[1][1], t[2][2]), glm::vec3{2, 3, 1}));
        }

        SUBCASE("Multiple calls don't stack")
        {
            transform.set_scale({2, 3});
            auto t = transform.get();
            DOCTEST_CHECK(approx_equal(glm::vec3(t[0][0], t[1][1], t[2][2]), glm::vec3{2, 3, 1}));
            transform.set_scale({-1, 2});
            t = transform.get();
            DOCTEST_CHECK(approx_equal(glm::vec3(t[0][0], t[1][1], t[2][2]), glm::vec3{-1, 2, 1}));
        }
    }
}
}