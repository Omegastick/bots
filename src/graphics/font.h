#pragma once

#include <memory>
#include <string>

#include "graphics/backend/texture.h"
#include "third_party/stb_truetype.h"

namespace SingularityTrainer
{
class Font
{
  private:
    std::unique_ptr<Texture> atlas_texture;
    std::vector<stbtt_packedchar> char_info;

  public:
    Font(const std::string &filepath, float size);

    void bind() const;
    void unbind() const;

    inline const std::vector<stbtt_packedchar> &get_char_info() const { return char_info; }
};
}