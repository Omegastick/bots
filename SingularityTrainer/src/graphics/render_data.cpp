#include <vector>

#include <spdlog/spdlog.h>

#include "graphics/render_data.h"
#include "graphics/sprite.h"

namespace SingularityTrainer
{
void RenderData::append(const std::vector<Sprite> &sprites)
{
    this->sprites.insert(this->sprites.end(), sprites.begin(), sprites.end());
}

void RenderData::append(const std::vector<Particle> &particles)
{
    this->particles.insert(this->particles.end(), particles.begin(), particles.end());
}

void RenderData::append(const std::vector<Line> &lines)
{
    this->lines.insert(this->lines.end(), lines.begin(), lines.end());
}

void RenderData::append(const std::vector<Text> &texts)
{
    this->texts.insert(this->texts.end(), texts.begin(), texts.end());
}

void RenderData::append(const RenderData &render_data)
{
    append(render_data.sprites);
    append(render_data.particles);
    append(render_data.lines);
    append(render_data.texts);
}
}