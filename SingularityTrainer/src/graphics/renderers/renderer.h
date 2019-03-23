#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "graphics/colors.h"

namespace SingularityTrainer
{
class PostProcLayer;
class FrameBuffer;
class ResourceManager;
class VertexArray;
class ElementBuffer;
class Shader;
class Sprite;
class Text;
class RenderData;
class SpriteRenderer;
class ParticleRenderer;
class LineRenderer;
class TextRenderer;

class Renderer
{
  private:
    int width, height;
    ResourceManager *resource_manager;
    std::unique_ptr<SpriteRenderer> sprite_renderer;
    std::unique_ptr<ParticleRenderer> particle_renderer;
    std::unique_ptr<LineRenderer> line_renderer;
    std::unique_ptr<TextRenderer> text_renderer;
    std::vector<PostProcLayer *> post_proc_layers;
    std::unique_ptr<FrameBuffer> base_frame_buffer;
    std::unique_ptr<FrameBuffer> texture_frame_buffer;

  public:
    Renderer(int width, int height, ResourceManager &resource_manager);
    ~Renderer();

    void resize(int width, int height);
    inline int get_width() const { return width; }
    inline int get_height() const { return height; }

    void draw(const VertexArray &vertex_array, const ElementBuffer &element_buffer, const Shader &shader);
    void draw(const Sprite &sprite, const glm::mat4 &view);
    void draw(const Text &text, const glm::mat4 &view);
    void draw(RenderData &render_data, const glm::mat4 &view, float time, bool lightweight = false);

    void clear(const glm::vec4 &color = cl_background);

    void push_post_proc_layer(PostProcLayer *post_proc_layer);
    void pop_post_proc_layer();
    void clear_post_proc_stack();

    void scissor(float x, float y, float width, float height, const glm::mat4 &projection) const;
    void clear_scissor() const;

    void begin();
    void end();
};
}