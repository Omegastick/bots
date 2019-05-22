#include <Box2D/Box2D.h>

#include <doctest/doctest.h>
#include <glm/glm.hpp>

#include "graphics/idrawable.h"
#include "graphics/render_data.h"
#include "training/modules/base_module.h"
#include "training/modules/imodule.h"
#include "training/modules/module_link.h"
#include "utilities.h"

namespace SingularityTrainer
{
ModuleLink::ModuleLink(float x, float y, float rot, IModule *parent)
    : is_parent(false), linked(false), visible(false), parent_module(parent)
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
    is_parent = true;
    linked = true;
    linked_module = other->parent_module;
    pair_link = other;
    other->is_parent = false;
    other->linked = true;
    other->linked_module = parent_module;
    other->pair_link = this;
}

b2Transform ModuleLink::get_global_transform() const
{
    return b2Mul(parent_module->get_global_transform(), transform);
}

RenderData ModuleLink::get_render_data(bool /*lightweight*/) { return RenderData(); }

TEST_CASE("ModuleLink")
{
    SUBCASE("Returns correct location for global transform")
    {
        BaseModule module;
        module.get_transform().p = {2, 3};
        module.get_transform().q = b2Rot(glm::radians(-90.f));

        auto module_link = module.get_module_links()[1];
        auto global_transform = module_link.get_global_transform();

        CHECK(global_transform.p.x == doctest::Approx(2));
        CHECK(global_transform.p.y == doctest::Approx(2.5));
        CHECK(global_transform.q.s == doctest::Approx(0));
        CHECK(global_transform.q.c == doctest::Approx(1));
    }
}
}