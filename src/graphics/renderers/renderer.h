#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "graphics/colors.h"
#include "graphics/render_data.h"
#include "third_party/di.hpp"

namespace SingularityTrainer
{
class DistortionLayer;
class PostProcLayer;
class FrameBuffer;
class ResourceManager;
class VertexArray;
class ElementBuffer;
class Shader;
struct Sprite;
struct Text;
class BatchedSpriteRenderer;
class ParticleRenderer;
class LineRenderer;
class TextRenderer;
class VectorRenderer;

static auto ResolutionX = [] {};
static auto ResolutionY = [] {};

class Renderer
{
  private:
    struct PackedSprite
    {
        unsigned int texture;
        glm::vec4 color;
        glm::mat4 transform;
    };

    int width, height;

    glm::mat4 view;

    std::vector<PostProcLayer *> post_proc_layers;
    std::unique_ptr<FrameBuffer> texture_frame_buffer;

    std::vector<std::string> textures;

    std::vector<Line> lines;
    std::vector<Particle> particles;
    std::vector<PackedSprite> sprites;
    std::vector<Text> texts;

    ResourceManager &resource_manager;

    BatchedSpriteRenderer &sprite_renderer;
    ParticleRenderer &particle_renderer;
    LineRenderer &line_renderer;
    TextRenderer &text_renderer;
    VectorRenderer &vector_renderer;

    DistortionLayer *distortion_layer;

  public:
    BOOST_DI_INJECT(Renderer,
                    (named = ResolutionX) int width,
                    (named = ResolutionY) int height,
                    ResourceManager &resource_manager,
                    BatchedSpriteRenderer &sprite_renderer,
                    ParticleRenderer &particle_renderer,
                    LineRenderer &line_renderer,
                    TextRenderer &text_renderer,
                    VectorRenderer &vector_renderer);

    void resize(int width, int height);

    void draw(const Line &line);
    void draw(const std::vector<Particle> &particles);
    void draw(const Sprite &sprite);
    void draw(const Text &text);
    void draw(const Circle &circle);
    void draw(const Rectangle &rectangle);
    void draw(const SemiCircle &semicircle);
    void draw(const Trapezoid &trapezoid);

    void clear(const glm::vec4 &color = cl_background);

    void push_post_proc_layer(PostProcLayer &post_proc_layer);
    void pop_post_proc_layer();
    void clear_post_proc_stack();

    void scissor(float x, float y, float width, float height, const glm::mat4 &projection) const;
    void clear_scissor() const;

    void clear_particles();

    void begin();
    void render(double time);

    void set_view(const glm::mat4 &view);

    inline int get_width() const { return width; }
    inline int get_height() const { return height; }

    void apply_explosive_force(glm::vec2 position, float size, float strength);
    void apply_implosive_force(glm::vec2 position, float size, float strength);
    inline void clear_distortion_layer() { distortion_layer = nullptr; }
    void set_distortion_layer(DistortionLayer &distortion_layer);
};
}