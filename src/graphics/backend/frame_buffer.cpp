#include <stdexcept>

#include <fmt/format.h>
#include <glad/glad.h>

#include "graphics/backend/frame_buffer.h"
#include "graphics/backend/texture.h"

namespace ai
{
FrameBuffer::FrameBuffer() : id(-1), render_buffer(-1)
{
    glGenFramebuffers(1, &id);
}

FrameBuffer &FrameBuffer::operator=(FrameBuffer &&other)
{
    if (this != &other)
    {
        id = other.id;
        other.id = 0;
        render_buffer = other.render_buffer;
        other.render_buffer = 0;
        texture = std::move(other.texture);
    }
    return *this;
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
    if (glIsRenderbuffer(render_buffer))
    {
        glDeleteRenderbuffers(1, &render_buffer);
    }

    glGenRenderbuffers(1, &render_buffer);
    bind();
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisampling, GL_RGBA16F, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER,
                              render_buffer);

    const auto framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error(fmt::format("Failed to initialize render buffer: {}",
                                             framebuffer_status)
                                     .c_str());
    }
}

void FrameBuffer::set_texture(int width, int height, bool use_stencil_buffer)
{
    bind();
    texture = std::make_unique<Texture>(width, height);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           texture->get_id(),
                           0);

    if (use_stencil_buffer)
    {
        stencil_buffer = std::make_unique<Texture>(width,
                                                   height,
                                                   GL_DEPTH24_STENCIL8,
                                                   GL_DEPTH_STENCIL,
                                                   GL_UNSIGNED_INT_24_8);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D,
                               stencil_buffer->get_id(),
                               0);
    }

    const auto framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error(fmt::format("Failed to initialize frame buffer object: {}",
                                             framebuffer_status)
                                     .c_str());
    }
}

glm::vec2 FrameBuffer::get_texture_size() const
{
    return glm::vec2(texture->get_width(), texture->get_height());
}
}