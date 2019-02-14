#include <vector>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/post_proc_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/shader.h"
#include "graphics/sprite.h"
#include "graphics/texture.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "iscreen.h"

namespace SingularityTrainer
{
PostProcScreen::PostProcScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    this->resource_manager = &resource_manager;
    resource_manager.load_texture("base_module", "images/base_module.png");
    sprite = std::make_unique<Sprite>(resource_manager.texture_store.get("base_module"));
    sprite->set_scale(glm::vec2(100, 100));
    sprite->set_origin(sprite->get_center());
    sprite->set_position(glm::vec2(960, 540));

    resource_manager.load_shader("texture", "shaders/texture.vert", "shaders/texture.frag");
    resource_manager.load_shader("post_proc_test", "shaders/texture.vert", "shaders/post_proc_test.frag");

    glGenFramebuffers(1, &fbo);
    glGenFramebuffers(1, &msfbo);
    glGenRenderbuffers(1, &rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, msfbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 1920, 1080);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Failed to initialize MSAA render buffer");
        throw std::exception();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    texture = std::make_shared<Texture>(1920, 1080);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->get_id(), 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Failed to initialize frame buffer object");
        throw std::exception();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

PostProcScreen::~PostProcScreen() {}

void PostProcScreen::update(const float delta_time)
{
    display_test_dialog("Post processing test", *screens, *screen_names, delta_time, *screen_manager);
    sprite->rotate(1.f * delta_time);
}

void PostProcScreen::draw(bool lightweight)
{
    Renderer renderer;
    auto shader = resource_manager->shader_store.get("texture");

    glBindFramebuffer(GL_FRAMEBUFFER, msfbo);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 mvp = projection * sprite->get_transform();
    shader->set_uniform_mat4f("u_mvp", mvp);
    sprite->get_texture().bind();
    shader->set_uniform_1i("u_texture", 0);

    renderer.draw(*sprite, *shader);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msfbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, 1920, 1080, 0, 0, 1920, 1080, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto post_proc_shader = resource_manager->shader_store.get("post_proc_test");
    texture->bind();
    post_proc_shader->set_uniform_1i("u_texture", 0);

    Sprite sprite(texture);
    sprite.set_scale(glm::vec2(1920.f, 1080.f));
    mvp = projection * sprite.get_transform();
    post_proc_shader->set_uniform_mat4f("u_mvp", mvp);
    post_proc_shader->set_uniform_2f("u_resolution", glm::vec2(1920, 1080));
    post_proc_shader->set_uniform_2f("u_direction", glm::vec2(1, 1));
    renderer.draw(sprite, *post_proc_shader);
}
}