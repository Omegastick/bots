#include <memory>

#include <glm/mat4x4.hpp>

#include "graphics/renderers/line_renderer.h"
#include "graphics/render_data.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/sprite.h"
#include "misc/resource_manager.h"

namespace SingularityTrainer
{
struct LineData
{
    glm::vec2 point;
    glm::vec2 normal;
    float miter;
    glm::vec4 color;
};

std::vector<LineData> get_polyline(const Line &line)
{
    glm::vec2 current_normal;
    std::vector<LineData> polyline;

    int quad_count = line.points.size();
    for (int i = 1; i < quad_count; ++i)
    {
        auto last_point = line.points[i - 1];
        auto current_point = line.points[i];
        auto next_point = i < quad_count - 1 ? line.points[i + 1] : glm::vec2();

        auto direction = current_point - last_point;
        direction = glm::normalize(direction);

        if (i == 1)
        {
            current_normal = glm::vec2(-direction.y, direction.x);
            polyline.push_back(LineData{
                line.points[i - 1],
                current_normal,
                line.widths[i - 1],
                line.colors[i - 1]});
            polyline.push_back(LineData{
                line.points[i - 1],
                current_normal,
                -line.widths[i - 1],
                line.colors[i - 1]});
        }

        if (i >= quad_count - 1)
        {
            current_normal = glm::vec2(-direction.y, direction.x);
            polyline.push_back(LineData{
                line.points[i],
                current_normal,
                line.widths[i],
                line.colors[i]});
            polyline.push_back(LineData{
                line.points[i],
                current_normal,
                -line.widths[i],
                line.colors[i]});
        }
        else
        {
            auto next_direction = next_point - current_normal;
            next_direction = glm::normalize(next_direction);

            auto tangent = direction + next_direction;
            tangent = glm::normalize(tangent);

            auto miter = glm::vec2(-tangent.y, tangent.x);
            auto normal = glm::vec2(-direction.y, direction.x);

            float miter_length = line.widths[i] / glm::dot(miter, normal);
            polyline.push_back(LineData{
                line.points[i],
                miter,
                miter_length,
                line.colors[i]});
            polyline.push_back(LineData{
                line.points[i],
                miter,
                -miter_length,
                line.colors[i]});
        }
    }

    return polyline;
}

LineRenderer::LineRenderer(ResourceManager &resource_manager)
    : resource_manager(resource_manager)
{
    vertex_array = std::make_unique<VertexArray>();
    vertex_buffer = std::make_unique<VertexBuffer>(nullptr, 0);
    element_buffer = std::make_unique<ElementBuffer>(nullptr, 0);

    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(2);
    layout.push<float>(1);
    layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, layout);

    resource_manager.load_shader("line", "shaders/line.vert", "shaders/default.frag");
}

void LineRenderer::draw(const Line &line, const glm::mat4 &view)
{
    vertex_array->bind();

    auto polyline = get_polyline(line);
    vertex_buffer->add_data(&polyline[0], sizeof(LineData) * polyline.size());

    int quad_count = line.points.size() - 1;
    std::vector<unsigned int> indices(quad_count * 6);
    for (int i = 0; i < quad_count * 2; ++i)
    {
        int index = i * 3;
        indices[index] = i + 0;
        indices[index + 1] = i + 1;
        indices[index + 2] = i + 2;
    }
    element_buffer->set_data(&indices[0], quad_count * 6);

    auto shader = resource_manager.shader_store.get("line");
    shader->set_uniform_mat4f("u_mvp", view);

    glDrawElements(GL_TRIANGLES, element_buffer->get_count(), GL_UNSIGNED_INT, 0);
}
}