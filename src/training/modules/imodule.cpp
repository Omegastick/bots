#include <vector>
#include <algorithm>

#include <glm/glm.hpp>

#include "graphics/render_data.h"
#include "graphics/renderers/renderer.h"
#include "training/modules/imodule.h"
#include "training/bodies/body.h"
#include "training/rigid_body.h"
#include "misc/utilities.h"

namespace SingularityTrainer
{
IModule::IModule()
    : body(nullptr),
      root(nullptr) {}

std::vector<IModule *> IModule::get_children()
{
    return get_children(std::vector<IModule *>());
}

std::vector<IModule *> IModule::get_children(std::vector<IModule *> child_list)
{
    if (std::find(child_list.begin(), child_list.end(), this) == child_list.end())
    {
        return child_list;
    }

    for (const auto &module_link : module_links)
    {
        if (module_link.linked)
        {
            module_link.linked_module->get_children(child_list);
        }
    }

    return child_list;
}

std::vector<float> IModule::get_sensor_reading() const { return std::vector<float>(); }

void IModule::draw(Renderer &renderer, bool /*lightweight*/)
{
    b2Transform world_transform = get_global_transform();
    glm::vec2 screen_position(world_transform.p.x, world_transform.p.y);
    sprite->transform.set_position(screen_position);
    auto rotation = world_transform.q.GetAngle();
    sprite->transform.set_rotation(rotation);

    renderer.draw(*sprite);
}

b2Transform IModule::get_global_transform() const
{
    if (body == nullptr)
    {
        return transform;
    }
    b2Body *b2_body = body->get_rigid_body().body;
    b2Transform body_transform = b2_body->GetTransform();

    return b2Mul(body_transform, transform);
}

void IModule::sub_update() {}

void IModule::update() {}
}