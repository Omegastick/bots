#include <vector>
#include <memory>
#include <string>
#include <fstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include "graphics/screens/text_test_screen.h"
#include "graphics/screens/test_utils.h"
#include "graphics/stb_truetype.h"
#include "graphics/sprite.h"
#include "graphics/backend/texture.h"
#include "graphics/backend/vertex_array.h"
#include "graphics/backend/vertex_buffer.h"
#include "graphics/backend/element_buffer.h"
#include "resource_manager.h"
#include "screen_manager.h"
#include "iscreen.h"

namespace SingularityTrainer
{
struct GlyphInfo
{
    glm::vec2 positions[4];
    glm::vec2 uvs[4];
    float offset_x = 0;
    float offset_y = 0;
};

GlyphInfo get_glyph_info(stbtt_packedchar *char_info, unsigned int character, float offset_x, float offset_y)
{
    stbtt_aligned_quad quad;

    stbtt_GetPackedQuad(char_info, 512, 512, character - 32, &offset_x, &offset_y, &quad, 1);
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

TextTestScreen::TextTestScreen(
    ScreenManager *screen_manager,
    ResourceManager &resource_manager,
    std::vector<std::shared_ptr<IScreen>> *screens,
    std::vector<std::string> *screen_names)
    : screens(screens), screen_names(screen_names), screen_manager(screen_manager), projection(glm::ortho(0.f, 1920.f, 0.f, 1080.f))
{
    this->resource_manager = &resource_manager;
    resource_manager.load_shader("font", "shaders/texture.vert", "shaders/font.frag");

    // Read file
    std::ifstream file("assets/fonts/Roboto-Regular.ttf", std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        spdlog::error("Failed to open file");
        throw std::exception();
    }
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    auto bytes = std::vector<uint8_t>(size);
    file.read(reinterpret_cast<char *>(&bytes[0]), size);
    file.close();

    unsigned char atlas_data[512 * 512];
    stbtt_packedchar char_info[96];

    stbtt_pack_context pack_context;
    if (!stbtt_PackBegin(&pack_context, atlas_data, 512, 512, 0, 1, nullptr))
    {
        spdlog::error("Failed to initialize font");
        throw std::exception();
    }

    stbtt_PackSetOversampling(&pack_context, 2, 2);
    if (!stbtt_PackFontRange(&pack_context, bytes.data(), 0, 24, 32, 96, char_info))
    {
        spdlog::error("Failed to pack font");
        throw std::exception();
    }

    stbtt_PackEnd(&pack_context);

    // Create atlas texture
    atlas_texture = std::make_unique<Texture>(512, 512, atlas_data);

    // Construct text vertices
    const std::string text = "Hello world!";

    std::vector<float> vertices;
    std::vector<glm::vec2> positions;
    std::vector<glm::vec2> tex_coords;
    std::vector<unsigned int> indices;

    unsigned int last_index = 0;
    float offset_x = 0;
    float offset_y = 0;
    for (auto character : text)
    {
        const auto glyphInfo = get_glyph_info(char_info, character, offset_x, offset_y);
        offset_x = glyphInfo.offset_x;
        offset_y = glyphInfo.offset_y;

        positions.emplace_back(glyphInfo.positions[0]);
        positions.emplace_back(glyphInfo.positions[1]);
        positions.emplace_back(glyphInfo.positions[2]);
        positions.emplace_back(glyphInfo.positions[3]);
        tex_coords.emplace_back(glyphInfo.uvs[0]);
        tex_coords.emplace_back(glyphInfo.uvs[1]);
        tex_coords.emplace_back(glyphInfo.uvs[2]);
        tex_coords.emplace_back(glyphInfo.uvs[3]);
        indices.push_back(last_index);
        indices.push_back(last_index + 1);
        indices.push_back(last_index + 2);
        indices.push_back(last_index);
        indices.push_back(last_index + 2);
        indices.push_back(last_index + 3);

        last_index += 4;
    }

    for (int i = 0; i < positions.size(); ++i)
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

    vertex_array = std::make_unique<VertexArray>();
    vertex_buffer = std::make_unique<VertexBuffer>(vertices.data(), sizeof(float) * vertices.size());
    element_buffer = std::make_unique<ElementBuffer>(indices.data(), indices.size());

    VertexBufferLayout vertex_buffer_layout;
    vertex_buffer_layout.push<float>(2);
    vertex_buffer_layout.push<float>(2);
    vertex_buffer_layout.push<float>(4);
    vertex_array->add_buffer(*vertex_buffer, vertex_buffer_layout);
}

TextTestScreen::~TextTestScreen() {}

void TextTestScreen::update(const float delta_time)
{
    display_test_dialog("Text test", *screens, *screen_names, delta_time, *screen_manager);
}

void TextTestScreen::draw(Renderer &renderer, bool lightweight)
{
    renderer.begin();

    auto shader = resource_manager->shader_store.get("font");
    auto mvp = glm::translate(projection, glm::vec3(500, 500, 0));
    shader->set_uniform_mat4f("u_mvp", mvp);
    shader->set_uniform_1i("u_texture", 0);
    atlas_texture->bind();

    renderer.draw(*vertex_array, *element_buffer, *shader);

    renderer.end();
}
}