#include <glm/gtc/matrix_transform.hpp>

#include "bloom_layer.h"
#include "graphics/backend/shader.h"
#include "misc/resource_manager.h"

namespace SingularityTrainer
{
BloomLayer::BloomLayer(ResourceManager &resource_manager, int width, int height)
    : PostProcLayer(*resource_manager.shader_store.get("bloom"), width, height),
      resource_manager(resource_manager)
{
    downsampled_fbo_1.set_texture(width / 2, height / 2);
    downsampled_fbo_2.set_texture(width / 4, height / 4);
    downsampled_fbo_3.set_texture(width / 8, height / 8);
    blurred_fbo_1.set_texture(width / 2, height / 2);
    blurred_fbo_2.set_texture(width / 4, height / 4);
    blurred_fbo_3.set_texture(width / 8, height / 8);
}

FrameBuffer &BloomLayer::render(Texture &input_texture)
{
    glDisable(GL_BLEND);

    // Apply highpass filter
    frame_buffer.bind();

    input_texture.bind();
    shader->set_uniform_1i("u_texture", 0);

    auto mvp = glm::mat4(1.0);
    shader->set_uniform_mat4f("u_mvp", mvp);

    glViewport(0, 0, width, height);
    vertex_array->bind();
    shader->bind();
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Downsample to 1/2 size
    glViewport(0, 0, width / 2, height / 2);
    downsampled_fbo_1.bind();
    frame_buffer.get_texture().bind();
    auto &texture_shader = *resource_manager.shader_store.get("texture");
    texture_shader.bind();
    texture_shader.set_uniform_1i("u_texture", 0);
    texture_shader.set_uniform_mat4f("u_mvp", mvp);
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Downsample to 1/4 size
    glViewport(0, 0, width / 4, height / 4);
    downsampled_fbo_2.bind();
    downsampled_fbo_1.get_texture().bind();
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Downsample to 1/8 size
    glViewport(0, 0, width / 8, height / 8);
    downsampled_fbo_3.bind();
    downsampled_fbo_2.get_texture().bind();
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Blur 1/8 size texture
    blurred_fbo_3.bind();
    downsampled_fbo_3.get_texture().bind();
    auto &blur_shader = *resource_manager.shader_store.get("blur");
    blur_shader.set_uniform_1i("u_texture", 0);
    blur_shader.set_uniform_mat4f("u_mvp", mvp);
    blur_shader.set_uniform_2f("u_offset", {0, 1. / (height / 8.)});
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    downsampled_fbo_3.bind();
    blurred_fbo_3.get_texture().bind();
    blur_shader.set_uniform_2f("u_offset", {1. / (width / 8.), 0});
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Combine 1/8 and 1/4 size textures
    glViewport(0, 0, width / 4, height / 4);
    downsampled_fbo_2.bind();
    downsampled_fbo_3.get_texture().bind(0);
    downsampled_fbo_2.get_texture().bind(1);
    auto &combine_shader = *resource_manager.shader_store.get("combine");
    combine_shader.bind();
    combine_shader.set_uniform_1i("u_texture_1", 0);
    combine_shader.set_uniform_1i("u_texture_2", 1);
    combine_shader.set_uniform_1f("u_amount_1", 1.f);
    combine_shader.set_uniform_1f("u_amount_2", 1.f);
    combine_shader.set_uniform_mat4f("u_mvp", mvp);
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Blur 1/4 size texture
    blurred_fbo_2.bind();
    downsampled_fbo_2.get_texture().bind();
    blur_shader.bind();
    blur_shader.set_uniform_2f("u_offset", {0, 1. / (height / 4.)});
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    downsampled_fbo_2.bind();
    blurred_fbo_2.get_texture().bind();
    blur_shader.set_uniform_2f("u_offset", {1. / (width / 4.), 0});
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Combine 1/4 and 1/2 size textures
    glViewport(0, 0, width / 2, height / 2);
    downsampled_fbo_1.bind();
    downsampled_fbo_2.get_texture().bind(0);
    downsampled_fbo_1.get_texture().bind(1);
    combine_shader.bind();
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Blur 1/2 size texture
    blurred_fbo_1.bind();
    downsampled_fbo_1.get_texture().bind();
    blur_shader.bind();
    blur_shader.set_uniform_2f("u_offset", {0, 1. / (height / 2.)});
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    downsampled_fbo_1.bind();
    blurred_fbo_1.get_texture().bind();
    blur_shader.set_uniform_2f("u_offset", {1. / (width / 2.), 0});
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    // Bring back up to full size
    glViewport(0, 0, width, height);
    frame_buffer.bind();
    downsampled_fbo_1.get_texture().bind(0);
    input_texture.bind(1);
    combine_shader.bind();
    combine_shader.set_uniform_1f("u_amount_1", 0.25f);
    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);

    glEnable(GL_BLEND);
    return frame_buffer;
}

void BloomLayer::resize(int width, int height)
{
    this->width = width;
    this->height = height;
    frame_buffer.set_texture(width, height);
    downsampled_fbo_1.set_texture(width / 2, height / 2);
    downsampled_fbo_2.set_texture(width / 4, height / 4);
    downsampled_fbo_3.set_texture(width / 8, height / 8);
    blurred_fbo_1.set_texture(width / 2, height / 2);
    blurred_fbo_2.set_texture(width / 4, height / 4);
    blurred_fbo_3.set_texture(width / 8, height / 8);
}
}