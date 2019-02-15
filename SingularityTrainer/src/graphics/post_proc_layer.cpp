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
PostProcLayer::PostProcLayer(Shader *shader) : shader(shader)
{
    vertex_array = std::make_unique<VertexArray>();

    float vertices[] = {
        -1, 1, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        -1, -1, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0,
        1, -1, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0,
        1, 1, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

    vertex_buffer = std::make_unique<VertexBuffer>(vertices, 4 * 8 * sizeof(float));

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0};

    element_buffer = std::make_unique<ElementBuffer>(indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);

    frame_buffer.add_texture(1920, 1080);
}

FrameBuffer &PostProcLayer::render(Texture &input_texture, Renderer &renderer)
{
    glDisable(GL_BLEND);
    frame_buffer.bind();

    input_texture.bind();
    shader->set_uniform_1i("u_texture", 0);

    auto mvp = glm::mat4(1.0);
    shader->set_uniform_mat4f("u_mvp", mvp);

    glViewport(0, 0, 1920, 1080);
    renderer.draw(*vertex_array, *element_buffer, *shader);

    glEnable(GL_BLEND);
    return frame_buffer;
}
}