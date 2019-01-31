#include <SFML/Graphics.hpp>

#include "gui/gui_button.h"

namespace SingularityTrainer
{
GUIButton::GUIButton(std::string label, float x, float y, float width, float height){};

GUIButton::~GUIButton() {}

void GUIButton::handle_input(sf::RenderWindow &window, const thor::ActionMap<Inputs> &action_map) {}

void GUIButton::draw(sf::RenderTarget &render_target) {}
}