#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/particle_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/backend/shader.h"
#include "graphics/renderers/renderer.h"
#include "graphics/renderers/particle_renderer.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "misc/resource_manager.h"
#include "misc/screen_manager.h"

namespace ai
{
ParticleTestScreen::ParticleTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens),
      screen_names(screen_names),
      screen_manager(screen_manager),
      resource_manager(&resource_manager),
      projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    resource_manager.load_shader("particle", "shaders/particle.vert", "shaders/default.frag");
}

void ParticleTestScreen::update(double delta_time)
{
    display_test_dialog("Particle test", *screens, *screen_names, delta_time, *screen_manager);
}

void ParticleTestScreen::draw(Renderer &renderer, bool /*lightweight*/)
{
    renderer.set_view(projection);
    std::vector<Particle> particles(10);
    for (int i = 0; i < 10; ++i)
    {
        Particle particle;
        particle.start_position = glm::vec2(960, 540);
        particle.velocity = glm::diskRand<float>(500);
        particle.start_time_offset = (float)i / 600;
        particle.lifetime = 5;
        particle.size = 1;
        particle.start_color = glm::vec4(1.0, 1.0, 1.0, 1.0);
        particle.end_color = glm::vec4(1.0, 1.0, 1.0, 0.0);
        particles.push_back(particle);
    }
    renderer.draw(particles);
}
}