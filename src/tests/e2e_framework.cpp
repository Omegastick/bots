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

void setup_imgui(double delta_time)
{
    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &imgui_io = ImGui::GetIO();
    imgui_io.DisplaySize = ImVec2(1920, 1080);
    imgui_io.DeltaTime = delta_time;
    ImFontConfig font_config;
    imgui_io.Fonts->ClearFonts();
    imgui_io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 16, &font_config);
    imgui_io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 24, &font_config);
    imgui_io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 32, &font_config);
    unsigned char *tex_pixels = NULL;
    int tex_w, tex_h;
    imgui_io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);
    imgui_io.IniFilename = NULL;
}
}