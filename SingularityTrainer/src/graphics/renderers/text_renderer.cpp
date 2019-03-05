#include <memory>

#include <glm/glm.hpp>

#include "graphics/render_data.h"
#include "graphics/renderers/text_renderer.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/element_buffer.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
struct GlyphInfo
{
    glm::vec2 positions[4];
    glm::vec2 uvs[4];
    float offset_x = 0;
    float offset_y = 0;
};

GlyphInfo get_glyph_info(const stbtt_packedchar *char_info, unsigned int character, float offset_x, float offset_y)
{
    stbtt_aligned_quad quad;

    stbtt_GetPackedQuad(char_info, 1024, 1024, character - 32, &offset_x, &offset_y, &quad, 1);
    auto x_min = quad.x0;
    auto x_max = quad.x1;
    auto y_min = -quad.y1;
    auto y_max = -quad.y0;

    GlyphInfo info;
    info.offset_x = offset_x;
    info.offset_y = offset_y;
    info.positions[0] = {x_min, y_min};
    info.positions[1] = {x_min, y_max};
    info.positions[2] = {x_max, y_max};
    info.positions[3] = {x_max, y_min};
    info.uvs[0] = {quad.s0, quad.t1};
    info.uvs[1] = {quad.s0, quad.t0};
    info.uvs[2] = {quad.s1, quad.t0};
    info.uvs[3] = {quad.s1, quad.t1};

    return info;
}

TextRenderer::TextRenderer(ResourceManager &resource_manager)
    : resource_manager(&resource_manager)
{
    vertex_array = std::make_unique<VertexArray>();
    vertex_buffer = std::make_unique<VertexBuffer>(nullptr, 0);
    element_buffer = std::make_unique<ElementBuffer>(nullptr, 0);

    VertexBufferLayout vertex_buffer_layout;
    vertex_buffer_layout.push<float>(2);
    vertex_buffer_layout.push<float>(2);
    vertex_buffer_layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, vertex_buffer_layout);

    resource_manager.load_shader("line", "shaders/line.vert", "shaders/default.frag");
}

void TextRenderer::draw(const Text &text, const glm::mat4 &view)
{
    auto font = resource_manager->font_store.get(text.font);

    vertex_array->bind();

    auto text_length = text.text.length();
    std::vector<float> vertices;
    vertices.reserve(text_length * 32);
    std::vector<glm::vec2> positions;
    positions.reserve(text_length * 4);
    std::vector<glm::vec2> tex_coords;
    tex_coords.reserve(text_length * 4);
    std::vector<unsigned int> indices;
    indices.reserve(text_length * 6);

    unsigned int last_index = 0;
    float offset_x = 0;
    float offset_y = 0;

    for (auto character : text.text)
    {
        const auto glyph_info = get_glyph_info(font->get_char_info().data(), character, offset_x, offset_y);
        offset_x = glyph_info.offset_x;
        offset_y = glyph_info.offset_y;

        positions.emplace_back(glyph_info.positions[0]);
        positions.emplace_back(glyph_info.positions[1]);
        positions.emplace_back(glyph_info.positions[2]);
        positions.emplace_back(glyph_info.positions[3]);
        tex_coords.emplace_back(glyph_info.uvs[0]);
        tex_coords.emplace_back(glyph_info.uvs[1]);
        tex_coords.emplace_back(glyph_info.uvs[2]);
        tex_coords.emplace_back(glyph_info.uvs[3]);
        indices.push_back(last_index);
        indices.push_back(last_index + 1);
        indices.push_back(last_index + 2);
        indices.push_back(last_index);
        indices.push_back(last_index + 2);
        indices.push_back(last_index + 3);

        last_index += 4;
    }

    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        vertices.push_back(positions[i].x);
        vertices.push_back(positions[i].y);
        vertices.push_back(tex_coords[i].x);
        vertices.push_back(tex_coords[i].y);
        vertices.push_back(1);
        vertices.push_back(1);
        vertices.push_back(1);
        vertices.push_back(1);
    }

    vertex_buffer->add_data(vertices.data(), sizeof(float) * vertices.size());
    element_buffer->set_data(indices.data(), indices.size());

    auto shader = resource_manager->shader_store.get("font");
    auto mvp = view * text.get_transform();
    shader->set_uniform_mat4f("u_mvp", mvp);
    shader->set_uniform_1i("u_texture", 0);
    shader->bind();
    font->bind();

    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);
}
}