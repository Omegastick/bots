#include <exception>

#include <glad/glad.h>
#include <spdlog/spdlog.h>

#include "graphics/frame_buffer.h"
#include "graphics/texture.h"

namespace SingularityTrainer
{
FrameBuffer::FrameBuffer() : id(0), render_buffer(0)
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

void FrameBuffer::add_render_buffer(int width, int height, int multisampling)
{
    if (render_buffer != 0)
    {
        spdlog::error("Render buffer already registered");
        throw std::exception();
    }

    glGenRenderbuffers(1, &render_buffer);
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

void FrameBuffer::add_texture(int width, int height)
{
    if (texture.get() != nullptr)
    {
        spdlog::error("Texture already registered");
        throw std::exception();
    }

    bind();
    texture = std::make_unique<Texture>(1920, 1080);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->get_id(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        spdlog::error("Failed to initialize frame buffer object");
        throw std::exception();
    }
}
}