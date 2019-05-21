#pragma once

#include <glm/glm.hpp>

namespace SingularityTrainer
{
class IO
{
  private:
    double cursor_x, cursor_y;
    int resolution_x, resolution_y;
    bool left_clicked;
    bool right_clicked;

  public:
    IO();

    glm::dvec2 get_cursor_position();
    bool get_left_click();
    glm::ivec2 get_resolution();
    bool get_right_click();
    void set_cursor_position(double cursor_x, double cursor_y);
    void set_left_click(bool state);
    void set_resolution(int resolution_x, int resolution_y);
    void set_right_click(bool state);
};
}