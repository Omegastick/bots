#pragma once

#include <vector>
#include <memory>

#include "graphics/vertex_array.h"
#include "graphics/shader.h"
#include "graphics/element_buffer.h"
#include "graphics/shader.h"
#include "graphics/sprite.h"
#include "graphics/post_proc_layer.h"
#include "graphics/frame_buffer.h"

namespace SingularityTrainer
{
class Renderer
{
  private:
    std::vector<PostProcLayer *> post_proc_layers;
    std::unique_ptr<FrameBuffer> base_frame_buffer;
    std::unique_ptr<FrameBuffer> texture_frame_buffer;
    int width, height;

  public:
    Renderer(int width, int height);
    ~Renderer();

    void resize(int width, int height);

    void draw(const VertexArray &vertex_array, const ElementBuffer &element_buffer, const Shader &shader);
    void draw(const Sprite &sprite, const Shader &shader);

    void clear();

    void push_post_proc_layer(PostProcLayer *post_proc_layer);
    void pop_post_proc_layer();
    void clear_post_proc_stack();

    void begin();
    void end();
};
}