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

void ModuleLink::link(ModuleLink &other)
{
    other.snap_to_other(*this);

    // Set linked flags and pointers
    is_parent = true;
    linked = true;
    linked_module = other.parent_module;
    pair_link = &other;
    other.is_parent = false;
    other.linked = true;
    other.linked_module = parent_module;
    other.pair_link = this;
}

void ModuleLink::snap_to_other(ModuleLink &other)
{
    // Calculate other module's transform
    b2Transform other_transform = b2Mul(other.parent_module->get_transform(), other.transform);

    // Calculate own transform
    b2Rot inverse_rotation = b2Mul(other_transform.q, b2Rot(b2_pi));
    parent_module->get_transform().q = b2Rot(inverse_rotation.GetAngle() - transform.q.GetAngle());
    parent_module->get_transform().p = b2Mul(parent_module->get_transform().q, transform.p);
    parent_module->get_transform().p *= -1;
    parent_module->get_transform().p += other_transform.p;
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