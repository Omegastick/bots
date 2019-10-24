#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <spdlog/spdlog.h>

#include "distortion_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/distortion_layer.h"
#include "graphics/renderers/renderer.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"
#include "screens/iscreen.h"

namespace SingularityTrainer
{
const int width = 1920;
const int height = 1080;

struct Vertex
{
    glm::vec2 position;
    glm::vec2 tex_coord;
    glm::vec4 color;
};

DistortionTestScreen::DistortionTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : resource_manager(resource_manager),
      screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    resource_manager.load_shader("distortion",
                                 "shaders/distortion.vert",
                                 "shaders/distortion.frag");
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>();
    sprite->texture = "base_module";
    sprite->transform.set_scale(glm::vec2(100, 100));
    sprite->transform.set_position(glm::vec2(960, 540));

    distortion_layer = std::make_unique<DistortionLayer>(
        *resource_manager.shader_store.get("distortion"),
        width, height, -0.1);
}

void DistortionTestScreen::update(double delta_time)
{
    display_test_dialog("Distortion test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->transform.rotate(1.f * static_cast<float>(delta_time));

    if (ImGui::IsKeyPressed(GLFW_KEY_Q))
    {
        distortion_layer->apply_explosive_force({96, 54}, 3, 10);
    }
    if (ImGui::IsKeyPressed(GLFW_KEY_W))
    {
        distortion_layer->apply_implosive_force({96, 54}, 5, 0.2);
    }

    distortion_layer->update_mesh();
}

void DistortionTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.set_view(projection);
    renderer.push_post_proc_layer(*distortion_layer);

    sprite->transform.set_position({860, 440});
    renderer.draw(*sprite);

    sprite->transform.set_position({960, 540});
    renderer.draw(*sprite);

    sprite->transform.set_position({1060, 640});
    renderer.draw(*sprite);

    sprite->transform.set_position({1060, 440});
    renderer.draw(*sprite);

    sprite->transform.set_position({860, 640});
    renderer.draw(*sprite);
}
}