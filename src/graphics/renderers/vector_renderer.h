#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "graphics/render_data.h"

class NVGcontext;

namespace ai
{
class VectorRenderer
{
  private:
    NVGcontext *vg;
    glm::vec2 resolution;

  public:
    VectorRenderer();
    ~VectorRenderer();

    void begin_frame(glm::vec2 resolution);
    void draw(const Circle &circle);
    void draw(const Rectangle &rectangle);
    void draw(const SemiCircle &semicircle);
    void draw(const Trapezoid &trapezoid);
    void end_frame();
    void set_view(const glm::mat4 &view);
};
}