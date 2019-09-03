#include <imgui.h>

#include "e2e_framework.h"
#include "misc/screen_manager.h"

namespace SingularityTrainer
{
Frame::Frame(double delta_time, ScreenManager &screen_manager)
    : delta_time(delta_time),
      screen_manager(screen_manager)
{
    ImGui::NewFrame();
}

Frame::~Frame()
{
    screen_manager.update(delta_time);
    ImGui::Render();
}
}