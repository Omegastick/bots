#pragma once

#include <SFML/Graphics.hpp>

#include "graphics/idrawable.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
class ScoreDisplay : IDrawable
{
  public:
    ScoreDisplay(float x, float y, float smoothing_weight, ResourceManager &resource_manager);
    ~ScoreDisplay();

    void add_score(float score);
    RenderData get_render_data(bool lightweight = false);

  private:
    sf::Text text;
    float running_score;
    float smoothing_weight;
};
}