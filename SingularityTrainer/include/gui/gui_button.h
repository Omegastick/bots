#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

#include "gui/igui_object.h"

namespace SingularityTrainer
{
class GUIButton : IGUIObject
{
  public:
    GUIButton(std::string text, float x, float y, float width, float height);
    ~GUIButton();

    void handle_input(sf::RenderWindow &window, const thor::ActionMap<Inputs> &action_map);
    void draw(sf::RenderTarget &render_target);

    std::function<void()> callback;
    sf::RectangleShape shape;
};
}