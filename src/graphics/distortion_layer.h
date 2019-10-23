#pragma once

#include <glm/glm.hpp>

#include "graphics/backend/element_buffer.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/post_proc_layer.h"
#include "misc/spring_mesh.h"

namespace SingularityTrainer
{
class Shader;

class DistortionLayer : public PostProcLayer
{
  private:
    int mesh_width, mesh_height;
    float scaling_factor;
    SpringMesh spring_mesh;
    std::unique_ptr<Texture> texture;

  public:
    DistortionLayer(Shader &shader,
                    int width = 1920,
                    int height = 1080,
                    float scaling_factor = 0.1f);
    DistortionLayer &operator=(DistortionLayer &&other) = delete;

    void apply_explosive_force(glm::vec2 position, float size, float strength);
    FrameBuffer &render(Texture &input_texture) override;
    void update_mesh();
    void update_texture();
};
}