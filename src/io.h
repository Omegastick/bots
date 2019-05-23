#pragma once

#include <glm/glm.hpp>

namespace SingularityTrainer
{
class IO
{
  private:
    double cursor_x, cursor_y;
    int resolution_x, resolution_y;
    bool left_clicked, right_clicked;
    bool keys[1024];
    bool keys_this_frame[1024];

  public:
    IO();

    glm::dvec2 get_cursor_position();
    bool get_key_pressed(int key);
    bool get_key_pressed_this_frame(int key);
    bool get_left_click();
    glm::ivec2 get_resolution();
    bool get_right_click();
    void left_click();
    void press_key(int key);
    void release_key(int key);
    void set_cursor_position(double cursor_x, double cursor_y);
    void right_click();
    void set_resolution(int resolution_x, int resolution_y);
    void tick();
};
}