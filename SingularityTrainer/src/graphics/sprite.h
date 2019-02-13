#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "graphics/vertex_array.h"
#include "graphics/vertex_buffer.h"
#include "graphics/texture.h"
#include "graphics/element_buffer.h"
#include "graphics/itransformable.h"
#include "idrawable.h"

namespace SingularityTrainer
{
struct Vertex
{
    glm::vec2 position;
    glm::vec2 texture_coord;
    glm::vec4 color;
};

class Sprite : public ITransformable
{
  private:
    std::unique_ptr<VertexArray> vertex_array;
    std::unique_ptr<VertexBuffer> vertex_buffer;
    std::unique_ptr<ElementBuffer> element_buffer;
    std::shared_ptr<Texture> texture;

  public:
    explicit Sprite(const std::shared_ptr<Texture> &texture);
    ~Sprite();

    glm::vec2 get_center() const;

    inline const VertexArray &get_vertex_array() const { return *vertex_array; }
    inline const VertexBuffer &get_vertex_buffer() const { return *vertex_buffer; }
    inline const ElementBuffer &get_element_buffer() const { return *element_buffer; }
    inline const Texture &get_texture() const { return *texture; }
};
}