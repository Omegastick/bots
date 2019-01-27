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

void IModule::draw(sf::RenderTarget &render_target)
{
    b2Vec2 world_position = get_global_position();
    sf::Vector2f screen_position(world_position.x, world_position.y);
    sprite.setPosition(screen_position);

    sprite.setRotation(rad_to_deg(agent->rigid_body->body->GetAngle() + rotation_rad));
    render_target.draw(sprite);
}

b2Vec2 IModule::get_global_position()
{
    b2Body *body = agent->rigid_body->body;
    b2Transform agent_transform = body->GetTransform();

    return agent_transform.p + rotate_point_around_point(b2Vec2(transform.p.x, transform.p.y), agent_transform.q, b2Vec2_zero);
}

void IModule::update() {}
}