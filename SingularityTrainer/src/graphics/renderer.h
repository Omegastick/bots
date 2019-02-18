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
#include "resource_manager.h"

namespace SingularityTrainer
{
struct SpriteVertex
{
    glm::vec2 position;
    glm::vec2 texture_coord;
    glm::vec4 color;
};

class Renderer
{
  private:
    int width, height;
    std::vector<PostProcLayer *> post_proc_layers;
    std::unique_ptr<FrameBuffer> base_frame_buffer;
    std::unique_ptr<FrameBuffer> texture_frame_buffer;
    std::unique_ptr<VertexArray> sprite_vertex_array;
    std::unique_ptr<VertexBuffer> sprite_vertex_buffer;
    std::unique_ptr<ElementBuffer> sprite_element_buffer;
    ResourceManager *resource_manager;

  public:
    Renderer(int width, int height, ResourceManager &resource_manager);
    ~Renderer();

    void resize(int width, int height);
    inline int get_width() const { return width; }
    inline int get_height() const { return height; }

    void draw(const VertexArray &vertex_array, const ElementBuffer &element_buffer, const Shader &shader);
    void draw(const Sprite &sprite, const glm::mat4 &transform);

    void clear();

    void push_post_proc_layer(PostProcLayer *post_proc_layer);
    void pop_post_proc_layer();
    void clear_post_proc_stack();

    void begin();
    void end();
};
}