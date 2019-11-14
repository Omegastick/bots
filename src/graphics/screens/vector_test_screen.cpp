#include <vector>
#include <memory>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nanovg.h>
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>
#include <imgui.h>

#include "vector_test_screen.h"
#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "graphics/screens/test_utils.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
VectorTestScreen::VectorTestScreen(
    ScreenManager &screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> &screens,
    std::vector<std::string> &screen_names)
    : resource_manager(resource_manager),
      screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    vg = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
}

VectorTestScreen::~VectorTestScreen()
{
    nvgDeleteGL3(vg);
}

void VectorTestScreen::update(double delta_time)
{
    display_test_dialog("Vector test", screens, screen_names, delta_time, screen_manager);

    rotation += delta_time * 0.1;
}

void VectorTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.set_view(projection);

    nvgBeginFrame(vg, renderer.get_width(), renderer.get_height(), 1);

    nvgBeginPath(vg);

    nvgTranslate(vg, 960, 540);
    nvgRotate(vg, static_cast<float>(rotation));

    nvgRect(vg, -100, -150, 200, 300);

    nvgRect(vg, -20, -30, 40, 60);
    nvgPathWinding(vg, NVG_HOLE);

    nvgFillColor(vg, nvgRGBA(128, 255, 64, 255));
    nvgFill(vg);

    nvgEndFrame(vg);
}
}