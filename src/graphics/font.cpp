#include <memory>
#include <string>
#include <vector>
#include <fstream>

#include <spdlog/spdlog.h>

#include "graphics/font.h"
#include "graphics/backend/texture.h"
#include "third_party/stb_truetype.h"

namespace SingularityTrainer
{
Font::Font(const std::string &filepath, float size)
{
    // Read file
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        spdlog::error("Failed to open file");
        throw std::exception();
    }
    const auto file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    auto bytes = std::vector<unsigned char>(file_size);
    file.read(reinterpret_cast<char *>(&bytes[0]), file_size);
    file.close();

    std::vector<unsigned char> atlas_data;
    atlas_data.resize(1024 * 1024);
    char_info.resize(96);

    stbtt_pack_context pack_context;
    if (!stbtt_PackBegin(&pack_context, atlas_data.data(), 1024, 1024, 0, 1, nullptr))
    {
        spdlog::error("Failed to initialize font");
        throw std::exception();
    }

    stbtt_PackSetOversampling(&pack_context, 2, 2);
    if (!stbtt_PackFontRange(&pack_context, bytes.data(), 0, size, 32, 96, char_info.data()))
    {
        spdlog::error("Failed to pack font");
        throw std::exception();
    }

    stbtt_PackEnd(&pack_context);

    // Create atlas texture
    atlas_texture = std::make_unique<Texture>(1024, 1024, atlas_data.data());
}

void Font::bind() const
{
    atlas_texture->bind();
}

void Font::unbind() const
{
    atlas_texture->unbind();
}
}