#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <math.h>

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
    // Calculate own transform relative to rigidbody
    b2Vec2 position = transform.p + parent_module->transform.p;
    b2Rot rotation = b2Mul(transform.q, parent_module->transform.q);

    // Calculate other's transform relative to rigidbody
    rotation = b2Mul(rotation, b2Rot(M_PI));
    
    // Calculate other's parent's transform
    b2Rot other_rot_inverse = b2Rot(-atan2(other->transform.q.s, other->transform.q.c));
    b2Vec2 other_pos_inverse = b2Vec2_zero - other->transform.p;
    other->parent_module->transform.p += position + rotate_point_around_point(other_pos_inverse, b2Mul(other->transform.q, transform.q), b2Vec2_zero);
    other->parent_module->transform.q = b2Mul(rotation, other_rot_inverse);
    other->parent_module->rotation_rad = atan2f(other->parent_module->transform.q.s, other->parent_module->transform.q.c);

    // Set linked flags and pointers
    linked = true;
    linked_module = other->parent_module;
    pair_link = other;
    other->linked = true;
    other->linked_module = parent_module;
    other->pair_link = this;
}

void ModuleLink::draw(sf::RenderTarget &render_target) {}
}