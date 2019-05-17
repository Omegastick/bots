#pragma once

#include <glm/glm.hpp>

namespace SingularityTrainer
{
class IO
{
  private:
    double cursor_x, cursor_y;
    int resolution_x, resolution_y;

  public:
    IO();

    glm::vec<2, double> get_cursor_position();
    glm::vec<2, int> get_resolution();
    void set_cursor_position(double cursor_x, double cursor_y);
    void set_resolution(int resolution_x, int resolution_y);
};
}