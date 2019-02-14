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
    sprite->set_scale(glm::vec2(1920.f, 1080.f));
    glm::mat4 mvp = projection * sprite->get_transform();
    shader->set_uniform_mat4f("u_mvp", mvp);
    shader->set_uniform_2f("u_resolution", glm::vec2(1920, 1080));
    shader->set_uniform_2f("u_direction", glm::vec2(1, 1));
    renderer.draw(*sprite, *shader);

    return frame_buffer;
}
}