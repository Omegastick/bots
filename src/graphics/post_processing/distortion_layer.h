#pragma once

#include <glm/glm.hpp>

#include "graphics/backend/element_buffer.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/post_processing/post_proc_layer.h"
#include "misc/spring_mesh.h"

namespace ai
{
class ResourceManager;

class DistortionLayer : public PostProcLayer
{
  private:
    int mesh_width, mesh_height;
    float scaling_factor;
    SpringMesh spring_mesh;
    std::unique_ptr<Texture> texture;

  public:
    DistortionLayer(ResourceManager &resource_manager,
                    int width = 1920,
                    int height = 1080,
                    float scaling_factor = -0.1f);

    void apply_explosive_force(glm::vec2 position, float size, float strength);
    void apply_implosive_force(glm::vec2 position, float size, float strength);
    FrameBuffer &render(Texture &input_texture) override;
    void update_mesh();
    void update_texture();

    glm::ivec2 get_size() const { return {mesh_width, mesh_height}; }
};
}