#include <memory>
#include <glm/glm.hpp>

#include "graphics/frame_buffer.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/sprite.h"
#include "graphics/renderer.h"
#include "graphics/post_proc_layer.h"

namespace SingularityTrainer
{
PostProcLayer::PostProcLayer(Shader *shader, const glm::mat4 &projection) : shader(shader), projection(projection)
{
    frame_buffer.add_texture(1920, 1080);
    sprite = std::make_unique<Sprite>(frame_buffer.get_texture());
}

FrameBuffer &PostProcLayer::render(Texture &input_texture, Renderer &renderer)
{
    frame_buffer.bind();

    input_texture.bind();
    shader->set_uniform_1i("u_texture", 0);

    sprite->set_texture(&input_texture);
    // sprite->set_scale(glm::vec2(1920, 1080));
    // sprite->set_position(glm::vec2(0, 0));
    // glm::mat4 mvp = projection * sprite->get_transform();
    // shader->set_uniform_mat4f("u_mvp", mvp);
    auto mvp = glm::mat4(1.0);
    shader->set_uniform_mat4f("u_mvp", mvp);
    // renderer.draw(*sprite, *shader);

    auto vertex_array = std::make_unique<VertexArray>();

    float vertices[] = {
        0, 1, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
        0, 0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0,
        1, 0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0,
        1, 1, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0};

    auto vertex_buffer = std::make_unique<VertexBuffer>(vertices, 4 * 8 * sizeof(float));

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    auto element_buffer = std::make_unique<ElementBuffer>(indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);

    renderer.draw(*vertex_array, *element_buffer, *shader);

    return frame_buffer;
}
}