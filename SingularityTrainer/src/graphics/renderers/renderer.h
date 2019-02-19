#pragma once

#include <vector>
#include <memory>

#include "graphics/renderers/sprite_renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/sprite.h"
#include "graphics/post_proc_layer.h"
#include "graphics/backend/frame_buffer.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
class Renderer
{
  private:
    int width, height;
    SpriteRenderer sprite_renderer;
    std::vector<PostProcLayer *> post_proc_layers;
    std::unique_ptr<FrameBuffer> base_frame_buffer;
    std::unique_ptr<FrameBuffer> texture_frame_buffer;
    ResourceManager *resource_manager;

  public:
    Renderer(int width, int height, ResourceManager &resource_manager);
    ~Renderer();

    void resize(int width, int height);
    inline int get_width() const { return width; }
    inline int get_height() const { return height; }

    void draw(const VertexArray &vertex_array, const ElementBuffer &element_buffer, const Shader &shader);
    void draw(const Sprite &sprite, const glm::mat4 &view);

    void clear();

    void push_post_proc_layer(PostProcLayer *post_proc_layer);
    void pop_post_proc_layer();
    void clear_post_proc_stack();

    void begin();
    void end();
};
}