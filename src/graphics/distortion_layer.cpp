#include <algorithm>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "distortion_layer.h"
#include "graphics/backend/element_buffer.h"
#include "graphics/backend/shader.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/vertex_buffer_layout.h"
#include "graphics/post_proc_layer.h"
#include "misc/spring_mesh.h"

namespace SingularityTrainer
{
DistortionLayer::DistortionLayer(Shader &shader, int width, int height)
    : PostProcLayer(shader, width, height),
      mesh_width(width * 0.1),
      mesh_height(height * 0.1),
      spring_mesh(mesh_width, mesh_height) {}

void DistortionLayer::apply_explosive_force(glm::vec2 position, float size, float strength)
{
    spring_mesh.apply_explosive_force(position, size, strength);
}

FrameBuffer &DistortionLayer::render(Texture &input_texture)
{
    update_texture();
    shader.set_uniform_1i("u_distortion", 1);
    texture->bind(1);

    return PostProcLayer::render(input_texture);
}

void DistortionLayer::update_mesh()
{
    spring_mesh.update();
}

void DistortionLayer::update_texture()
{
    std::vector<float> pixels(mesh_width * mesh_height * 2);
    auto &offsets = spring_mesh.get_offsets();
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        pixels[i * 2] = offsets[i].x;
        pixels[i * 2 + 1] = offsets[i].y;
    }

    float min = *std::min_element(pixels.begin(), pixels.end());
    float max = *std::max_element(pixels.begin(), pixels.end());
    spdlog::debug("Min: {}, Max: {}", min, max);

    texture = std::make_unique<Texture>(mesh_width, mesh_height, pixels.data());
}
}