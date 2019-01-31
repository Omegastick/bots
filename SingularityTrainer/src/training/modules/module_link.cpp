#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include "training/modules/imodule.h"
#include "training/modules/module_link.h"
#include "utilities.h"

namespace SingularityTrainer
{
ModuleLink::ModuleLink(float x, float y, float rot, IModule *parent)
    : parent_module(parent), linked(false), visible(false)
{
    transform.p = b2Vec2(x, y);
    transform.q = b2Rot(deg_to_rad(rot));
}
ModuleLink::~ModuleLink() {}

void ModuleLink::link(ModuleLink *other)
{
    // Calculate own transform
    b2Transform own_transform = b2Mul(parent_module->transform, transform);

    // Calculate other module's transform
    b2Rot inverse_rotation = b2Mul(own_transform.q, b2Rot(M_PI));
    other->parent_module->transform.q = b2Rot(inverse_rotation.GetAngle() - other->transform.q.GetAngle());
    other->parent_module->transform.p = b2Mul(other->parent_module->transform.q, other->transform.p);
    other->parent_module->transform.p *= -1;
    other->parent_module->transform.p += own_transform.p;

    // Set linked flags and pointers
    linked = true;
    linked_module = other->parent_module;
    pair_link = other;
    other->linked = true;
    other->linked_module = parent_module;
    other->pair_link = this;
}

void ModuleLink::draw(sf::RenderTarget &render_target, bool lightweight) {}
}