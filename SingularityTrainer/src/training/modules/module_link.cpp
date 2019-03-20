#include <Box2D/Box2D.h>

#include <glm/glm.hpp>

#include "graphics/idrawable.h"
#include "graphics/render_data.h"
#include "training/modules/imodule.h"
#include "training/modules/module_link.h"
#include "utilities.h"

namespace SingularityTrainer
{
ModuleLink::ModuleLink(float x, float y, float rot, IModule *parent)
    : parent_module(parent), linked(false), visible(false)
{
    transform.p = b2Vec2(x, y);
    transform.q = b2Rot(glm::radians(rot));
}
ModuleLink::~ModuleLink() {}

void ModuleLink::link(ModuleLink *other)
{
    // Calculate own transform
    b2Transform own_transform = b2Mul(parent_module->get_transform(), transform);

    // Calculate other module's transform
    b2Rot inverse_rotation = b2Mul(own_transform.q, b2Rot(b2_pi));
    other->parent_module->get_transform().q = b2Rot(inverse_rotation.GetAngle() - other->transform.q.GetAngle());
    other->parent_module->get_transform().p = b2Mul(other->parent_module->get_transform().q, other->transform.p);
    other->parent_module->get_transform().p *= -1;
    other->parent_module->get_transform().p += own_transform.p;

    // Set linked flags and pointers
    linked = true;
    linked_module = other->parent_module;
    pair_link = other;
    other->linked = true;
    other->linked_module = parent_module;
    other->pair_link = this;
}

RenderData ModuleLink::get_render_data(bool lightweight) { return RenderData(); }
}