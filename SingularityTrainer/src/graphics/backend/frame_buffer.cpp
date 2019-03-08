#include <exception>

#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include "graphics/backend/frame_buffer.h"
#include "graphics/backend/texture.h"

namespace SingularityTrainer
{
FrameBuffer::FrameBuffer() : id(-1), render_buffer(-1)
{
    glGenFramebuffers(1, &id);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &id);
    if (render_buffer != 0)
    {
        glDeleteRenderbuffers(1, &render_buffer);
    }
}

void FrameBuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void FrameBuffer::bind_read() const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
}

void FrameBuffer::bind_draw() const
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
}

void FrameBuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::set_render_buffer(int width, int height, int multisampling)
{
    auto x = render_buffer;
    if (glIsRenderbuffer(render_buffer))
    {
        glDeleteRenderbuffers(1, &render_buffer);
    }

    glGenRenderbuffers(1, &render_buffer);
    auto y = render_buffer;
    bind();
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampling, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Failed to initialize render buffer");
        throw std::exception();
    }
}

void FrameBuffer::set_texture(int width, int height)
{
    bind();
    texture = std::make_unique<Texture>(width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->get_id(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Failed to initialize frame buffer object");
        throw std::exception();
    }
}

glm::vec2 FrameBuffer::get_texture_size() const
{
    return glm::vec2(texture->get_width(), texture->get_height());
}
}