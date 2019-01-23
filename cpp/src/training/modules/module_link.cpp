#include <SFML/Graphics.hpp>

#include "training/modules/imodule.h"
#include "training/modules/module_link.h"

namespace SingularityTrainer
{
ModuleLink::ModuleLink(float x, float y, float rot) : x(x), y(y), rot(rot), linked(false), visible(false) {}
ModuleLink::~ModuleLink() {}

void ModuleLink::draw(sf::RenderTarget &render_target) {}
}