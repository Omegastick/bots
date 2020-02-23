#pragma once

#include "graphics/backend/frame_buffer.h"
#include "graphics/post_processing/post_proc_layer.h"

namespace ai
{
class ResourceManager;

class BloomLayer : public PostProcLayer
{
  private:
    FrameBuffer blurred_fbo_1, blurred_fbo_2, blurred_fbo_3;
    FrameBuffer combined_fbo_1, combined_fbo_2;
    FrameBuffer downsampled_fbo_1, downsampled_fbo_2, downsampled_fbo_3;
    ResourceManager &resource_manager;

  public:
    BloomLayer(ResourceManager &resource_manager, int width = 1920, int height = 1080);

    FrameBuffer &render(Texture &input_texture) override;
    void resize(int width, int height) override;
};
}