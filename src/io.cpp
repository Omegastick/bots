#include <doctest/doctest.h>
#include <glm/glm.hpp>

#include "io.h"

namespace SingularityTrainer
{
IO::IO() : cursor_x(0), cursor_y(0), resolution_x(0), resolution_y(0) {}

glm::vec<2, double> IO::get_cursor_position()
{
    return {cursor_x, cursor_y};
}

glm::vec<2, int> IO::get_resolution()
{
    return {resolution_x, resolution_y};
}

void IO::set_cursor_position(double cursor_x, double cursor_y)
{
    this->cursor_x = cursor_x;
    this->cursor_y = cursor_y;
}

void IO::set_resolution(int resolution_x, int resolution_y)
{
    this->resolution_x = resolution_x;
    this->resolution_y = resolution_y;
}

TEST_CASE("IO")
{
    IO io;

    SUBCASE("Resolution is initialized to 0")
    {
        auto resolution = io.get_resolution();
        CHECK(resolution.x == 0);
        CHECK(resolution.y == 0);
    }

    SUBCASE("Cursor position is initialized to 0")
    {
        auto cursor_pos = io.get_cursor_position();
        CHECK(cursor_pos.x == 0);
        CHECK(cursor_pos.y == 0);
    }

    SUBCASE("Setting and getting resolution works")
    {
        io.set_resolution(100, 80);
        auto resolution = io.get_resolution();
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
}
}