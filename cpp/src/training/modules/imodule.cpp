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
    b2Body *body = fixture->GetBody();
    b2Transform transform = body->GetTransform();

    b2PolygonShape *shape = (b2PolygonShape *)fixture->GetShape();

    b2Vec2 world_position = b2Mul(transform, shape->m_centroid);
    sf::Vector2f screen_position(world_position.x, world_position.y);
    sprite.setPosition(screen_position);
    sprite.setRotation(rad_to_deg(body->GetAngle()));
    render_target.draw(sprite);
}
}