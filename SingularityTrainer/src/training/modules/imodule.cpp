#include <algorithm>

#include "training/modules/imodule.h"
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

void IModule::draw(sf::RenderTarget &render_target, bool lightweight)
{
    b2Transform world_transform = get_global_transform();
    sf::Vector2f screen_position(world_transform.p.x, world_transform.p.y);
    sprite.setPosition(screen_position);

    sprite.setRotation(rad_to_deg(world_transform.q.GetAngle()));
    render_target.draw(sprite);
}

b2Transform IModule::get_global_transform()
{
    b2Body *body = agent->rigid_body->body;
    b2Transform agent_transform = body->GetTransform();

    return b2Mul(agent_transform, transform);
}

void IModule::update() {}
}