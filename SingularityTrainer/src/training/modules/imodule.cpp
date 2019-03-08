#include <vector>
#include <algorithm>

#include <glm/glm.hpp>

#include "graphics/render_data.h"
#include "training/modules/imodule.h"
#include "training/agents/iagent.h"
#include "utilities.h"

namespace SingularityTrainer
{
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

std::vector<float> IModule::get_sensor_reading() { return std::vector<float>(); }

RenderData IModule::get_render_data(bool lightweight)
{
    b2Transform world_transform = get_global_transform();
    glm::vec2 screen_position(world_transform.p.x, world_transform.p.y);
    sprite->set_position(screen_position);
    auto rotation = world_transform.q.GetAngle();
    sprite->set_rotation(rotation);

    return RenderData{{*sprite}};
}

b2Transform IModule::get_global_transform()
{
    b2Body *body = agent->rigid_body->body;
    b2Transform agent_transform = body->GetTransform();

    return b2Mul(agent_transform, transform);
}

void IModule::update() {}
}