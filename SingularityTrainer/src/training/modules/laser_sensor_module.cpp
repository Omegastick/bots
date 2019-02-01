#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "gui/colors.h"
#include "resource_manager.h"
#include "training/agents/iagent.h"
#include "training/modules/base_module.h"
#include "training/modules/imodule.h"
#include "training/modules/laser_sensor_module.h"
#include "training/modules/module_link.h"
#include "utilities.h"

namespace SingularityTrainer
{
LaserSensorModule::LaserSensorModule(ResourceManager &resource_manager, b2Body &body, IAgent *agent)
    : laser_count(9), fov(180), laser_length(10), time(0)
{
    // Sprite
    resource_manager.load_texture("laser_sensor_module", "images/laser_sensor_module.png");
    sprite.setTexture(*resource_manager.texture_store.get("laser_sensor_module"));
    sprite.setScale(0.01, 0.01);
    sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);
    sprite.setColor(cl_white);

    // Box2D
    b2PolygonShape shape;
    shape.SetAsBox(0.5, 0.25);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, 0.25, 180, this));

    this->agent = agent;

    // Shader
    resource_manager.load_shader("laser", "shaders/laser.frag");
    laser_shader = resource_manager.shader_store.get("laser");
}

LaserSensorModule::~LaserSensorModule() {}

std::vector<float> LaserSensorModule::get_sensor_reading()
{
    std::vector<float> sensor_reading;
    sensor_reading.resize(laser_count);

    b2Transform global_transform = get_global_transform();
    float segment_width = fov / (laser_count - 1);

    for (int i = 0; i < laser_count; ++i)
    {
        ClosestRaycastCallback raycast_callback;

        b2Rot angle(deg_to_rad((segment_width * i) - (fov / 2)));
        b2Vec2 laser = b2Mul(angle, b2Vec2(0, -laser_length));

        agent->rigid_body->body->GetWorld()->RayCast(&raycast_callback, global_transform.p, b2Mul(global_transform, laser));
        if (raycast_callback.distance == -1)
        {
            sensor_reading[i] = 1;
        }
        else
        {
            sensor_reading[i] = raycast_callback.distance;
        }
    }

    last_reading = sensor_reading;
    return sensor_reading;
}

void LaserSensorModule::draw(sf::RenderTarget &render_target, bool lightweight)
{
    if (!lightweight)
    {
        b2Transform global_transform = get_global_transform();
        float segment_width = fov / (laser_count - 1);

        sf::VertexArray vertices(sf::Lines, laser_count * 2);

        sf::Color color = cl_white;
        color.a = 100;

        for (int i = 0; i < laser_count; ++i)
        {
            if (last_reading[i] < 1)
            {
                b2Rot angle(deg_to_rad((segment_width * i) - (fov / 2)));
                b2Vec2 laser = b2Mul(angle, b2Vec2(0, -last_reading[i] * laser_length));
                b2Vec2 laser_start = b2Mul(angle, b2Vec2(0, -0.35));
                b2Vec2 transformed_end = b2Mul(global_transform, laser);
                b2Vec2 transformed_start = b2Mul(global_transform, laser_start);
                vertices[i * 2].position = sf::Vector2f(transformed_start.x, transformed_start.y);
                vertices[i * 2].color = color;
                vertices[i * 2 + 1].position = sf::Vector2f(transformed_end.x, transformed_end.y);
                vertices[i * 2 + 1].color = color;
            }
            else
            {
                vertices[i * 2].color = sf::Color::Transparent;
                vertices[i * 2 + 1].color = sf::Color::Transparent;
            }
        }

        laser_shader->setUniform("u_Time", time);
        time += 0.02;
        render_target.draw(vertices, laser_shader.get());
    }

    IModule::draw(render_target, lightweight);
}

ClosestRaycastCallback::ClosestRaycastCallback() : distance(-1) {}
ClosestRaycastCallback::~ClosestRaycastCallback() {}

float32 ClosestRaycastCallback::ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float32 fraction)
{
    distance = fraction;
    return fraction;
}
}
