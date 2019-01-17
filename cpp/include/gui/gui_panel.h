#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "gui/igui_object.h"
#include "idrawable.h"

namespace SingularityTrainer
{
class GUIPanel : IGUIObject
{
  public:
    GUIPanel(float x, float y, float width, float height);
    ~GUIPanel();

    void handle_input(sf::RenderWindow &window);
    void draw(sf::RenderTarget &render_target);

    sf::RectangleShape shape;
    std::vector<std::shared_ptr<IGUIObject>> children;
};
}