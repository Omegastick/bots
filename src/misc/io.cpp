#include <iterator>
#include <vector>

#include <doctest.h>
#include <glm/glm.hpp>

#include "misc/io.h"

namespace ai
{
IO::IO()
    : cursor_x(0), cursor_y(0),
      resolution_x(0), resolution_y(0),
      left_clicked(false), right_clicked(false),
      keys{false},
      keys_this_frame{false} {}

glm::dvec2 IO::get_cursor_position() const
{
    return {cursor_x, cursor_y};
}

bool IO::get_key_pressed(int key) const
{
    return keys[key];
}

bool IO::get_key_pressed_this_frame(int key) const
{
    return keys_this_frame[key];
}

bool IO::get_left_click() const
{
    return left_clicked;
}

glm::ivec2 IO::get_resolution() const
{
    return {resolution_x, resolution_y};
}

glm::vec2 IO::get_resolutionf() const
{
    return {static_cast<float>(resolution_x), static_cast<float>(resolution_y)};
}

bool IO::get_right_click() const
{
    return right_clicked;
}

void IO::left_click()
{
    left_clicked = true;
}

void IO::press_key(int key)
{
    keys[key] = true;
    keys_this_frame[key] = true;
}

void IO::release_key(int key)
{
    keys[key] = false;
}

void IO::set_cursor_position(double x, double y)
{
    cursor_x = x;
    cursor_y = y;
}

void IO::right_click()
{
    right_clicked = true;
}

void IO::set_resolution(int x, int y)
{
    resolution_x = x;
    resolution_y = y;
}

void IO::tick()
{
    left_clicked = false;
    right_clicked = false;
    std::fill(std::begin(keys_this_frame), std::end(keys_this_frame), false);
}

TEST_CASE("IO")
{
    IO io;

    SUBCASE("Resolution is initialized to 0")
    {
        auto resolution = io.get_resolutionf();

        CHECK(resolution.x == 0);
        CHECK(resolution.y == 0);
    }

    SUBCASE("Cursor position is initialized to 0")
    {
        auto cursor_pos = io.get_cursor_position();

        CHECK(cursor_pos.x == 0);
        CHECK(cursor_pos.y == 0);
    }

    SUBCASE("Mouse buttons are initialized to false")
    {
        CHECK(!io.get_left_click());
        CHECK(!io.get_right_click());
    }

    SUBCASE("Keyboard keys are initialized to unpressed (false)")
    {
        bool any_keys_pressed = false;
        for (int i = 0; i < 1024; ++i)
        {
            any_keys_pressed = any_keys_pressed || io.get_key_pressed(i);
        }

        CHECK(!any_keys_pressed);
    }

    SUBCASE("Setting and getting resolution works")
    {
        io.set_resolution(100, 80);
        auto resolution = io.get_resolutionf();

        CHECK(resolution.x == 100);
        CHECK(resolution.y == 80);
    }

    SUBCASE("Setting and getting cursor position works")
    {
        io.set_cursor_position(100, 80);
        auto cursor_pos = io.get_cursor_position();

        CHECK(cursor_pos.x == 100);
        CHECK(cursor_pos.y == 80);
    }

    SUBCASE("Setting and getting left click works")
    {
        io.left_click();

        CHECK(io.get_left_click());
    }

    SUBCASE("Setting and getting right click works")
    {
        io.right_click();

        CHECK(io.get_right_click());
    }

    SUBCASE("Pushing keys works")
    {
        io.press_key(5);

        CHECK(io.get_key_pressed(5));

        io.release_key(5);

        CHECK(!io.get_key_pressed(5));
    }
}
}