#include <glad/glad.h>
#include <glm/glm.hpp>
#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

#include "vector_renderer.h"
#include "graphics/render_data.h"

NVGcolor glm_to_nvg(const glm::vec4 color)
{
    return nvgRGBAf(color.r, color.g, color.b, color.a);
}

namespace SingularityTrainer
{
VectorRenderer::VectorRenderer()
{
    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
}

VectorRenderer::~VectorRenderer()
{
    nvgDeleteGL3(vg);
}

void VectorRenderer::begin_frame(glm::vec2 resolution)
{
    this->resolution = resolution;
    nvgBeginFrame(vg, resolution.x, resolution.y, 1.f);
}

void VectorRenderer::draw(const Circle &circle)
{
    nvgSave(vg);

    const auto position = circle.transform.get_position();
    nvgTranslate(vg, position.x, position.y);

    nvgBeginPath(vg);

    nvgCircle(vg, 0, 0, circle.radius);

    if (circle.fill_color.a != 0.f)
    {
        nvgFillColor(vg, glm_to_nvg(circle.fill_color));
        nvgFill(vg);
    }

    if (circle.stroke_width != 0.f)
    {
        nvgStrokeWidth(vg, circle.stroke_width);
        nvgStrokeColor(vg, glm_to_nvg(circle.stroke_color));
        nvgStroke(vg);
    }

    nvgRestore(vg);
}

void VectorRenderer::draw(const Rectangle &rectangle)
{
    nvgSave(vg);

    const auto position = rectangle.transform.get_position();
    nvgTranslate(vg, position.x, position.y);

    const auto rotation = rectangle.transform.get_rotation();
    nvgRotate(vg, rotation);

    nvgBeginPath(vg);

    const auto size = rectangle.transform.get_scale();
    nvgRect(vg, -size.x * 0.5f, -size.y * 0.5f, size.x, size.y);

    if (rectangle.fill_color.a != 0.f)
    {
        nvgFillColor(vg, glm_to_nvg(rectangle.fill_color));
        nvgFill(vg);
    }

    if (rectangle.stroke_width != 0.f)
    {
        nvgStrokeWidth(vg, rectangle.stroke_width);
        nvgStrokeColor(vg, glm_to_nvg(rectangle.stroke_color));
        nvgStroke(vg);
    }

    nvgRestore(vg);
}

void VectorRenderer::draw(const SemiCircle &semicircle) {}

void VectorRenderer::draw(const Trapezoid &trapezoid)
{
    nvgSave(vg);

    const auto position = trapezoid.transform.get_position();
    nvgTranslate(vg, position.x, position.y);

    const auto rotation = trapezoid.transform.get_rotation();
    nvgRotate(vg, rotation);

    nvgBeginPath(vg);

    const auto size = trapezoid.transform.get_scale();
    nvgMoveTo(vg, -trapezoid.bottom_width * 0.5 * size.x, size.y * 0.5);
    nvgLineTo(vg, trapezoid.bottom_width * 0.5 * size.x, size.y * 0.5);
    nvgLineTo(vg, trapezoid.top_width * 0.5 * size.x, -size.y * 0.5);
    nvgLineTo(vg, -trapezoid.top_width * 0.5 * size.x, -size.y * 0.5);
    nvgClosePath(vg);

    if (trapezoid.fill_color.a != 0.f)
    {
        nvgFillColor(vg, glm_to_nvg(trapezoid.fill_color));
        nvgFill(vg);
    }

    if (trapezoid.stroke_width != 0.f)
    {
        nvgStrokeWidth(vg, trapezoid.stroke_width);
        nvgStrokeColor(vg, glm_to_nvg(trapezoid.stroke_color));
        nvgStroke(vg);
    }

    nvgRestore(vg);
}

void VectorRenderer::end_frame() { nvgEndFrame(vg); }

void VectorRenderer::set_view(const glm::mat4 &view)
{
    nvgResetTransform(vg);
    nvgTransform(vg, 1, 0, 0, -1, 0, resolution.y);
    nvgScale(vg, view[0][0], view[1][1]);
    nvgScale(vg, resolution.x * 0.5f, resolution.y * 0.5f);
    nvgTranslate(vg, 1.f / view[0][0], 1.f / view[1][1]);
}
}
