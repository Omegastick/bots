#pragma once

#include <SFML/Graphics.hpp>

#include "idrawable.h"
#include "resource_manager.h"

namespace SingularityTrainer
{
class ScoreDisplay : IDrawable
{
  public:
    ScoreDisplay(float x, float y, float smoothing_weight, ResourceManager &resource_manager);
    ~ScoreDisplay();

    void add_score(float score);
    void draw(sf::RenderTarget &render_target, bool lightweight = false);

  private:
    sf::Text text;
    float running_score;
    float smoothing_weight;
};
}