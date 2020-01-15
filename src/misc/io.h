#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>

namespace ai
{
enum class MouseButton
{
    Left,
    Right
};

class IO
{
  private:
    std::vector<std::function<void(MouseButton)>> click_callbacks;
    double cursor_x, cursor_y;
    int resolution_x, resolution_y;
    bool left_clicked, right_clicked;
    bool keys[1024];
    bool keys_this_frame[1024];

  public:
    IO();

    void add_click_callback(std::function<void(MouseButton)> callback);
    glm::dvec2 get_cursor_position() const;
    bool get_key_pressed(int key) const;
    bool get_key_pressed_this_frame(int key) const;
    bool get_left_click() const;
    glm::ivec2 get_resolution() const;
    glm::vec2 get_resolutionf() const;
    bool get_right_click() const;
    void left_click();
    void press_key(int key);
    void release_key(int key);
    void set_cursor_position(double x, double y);
    void right_click();
    void set_resolution(int x, int y);
    void tick();
};
}