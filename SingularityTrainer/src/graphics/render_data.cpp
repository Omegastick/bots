#include <vector>

#include "graphics/render_data.h"
#include "graphics/sprite.h"

namespace SingularityTrainer
{
void RenderData::append(const std::vector<Sprite> &sprites, const std::vector<Particle> &particles)
{
    this->sprites.insert(sprites.end(), sprites.begin(), sprites.end());
    this->particles.insert(particles.end(), particles.begin(), particles.end());
}

void RenderData::append(const RenderData &render_data)
{
    append(render_data.sprites, render_data.particles);
}
}