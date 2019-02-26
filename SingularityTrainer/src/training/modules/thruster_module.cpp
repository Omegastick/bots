#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include <Box2D/Box2D.h>

#include "graphics/colors.h"
#include "resource_manager.h"
#include "training/actions/activate_action.h"
#include "training/agents/iagent.h"
#include "training/modules/imodule.h"
#include "training/modules/thruster_module.h"

namespace SingularityTrainer
{
ThrusterModule::ThrusterModule(b2Body &body, IAgent *agent)
    : active(false), particle_color(cl_white)
{
    // Sprite
    sprite = std::make_unique<Sprite>("thruster_module");
    sprite->set_scale(glm::vec2(1, 0.333));
    sprite->set_origin(sprite->get_center());

    // Box2D fixture
    b2Vec2 vertices[4];
    vertices[0] = b2Vec2(-0.333, -0.167);
    vertices[1] = b2Vec2(-0.5, 0.167);
    vertices[2] = b2Vec2(0.5, 0.167);
    vertices[3] = b2Vec2(0.333, -0.167);
    b2PolygonShape shape;
    shape.Set(vertices, 4);
    shapes.push_back(shape);
    transform.SetIdentity();

    // Module links
    module_links.push_back(ModuleLink(0, 0.167, 0, this));

    actions.push_back(std::make_unique<ActivateAction>(this));

    this->agent = agent;
}

ThrusterModule::~ThrusterModule() {}

RenderData ThrusterModule::get_render_data(bool lightweight)
{
    auto render_data = IModule::get_render_data(lightweight);

    // Spawn particles
    if (active && !lightweight)
    {
        b2Transform global_transform = get_global_transform();
        b2Transform edge_transform = b2Mul(global_transform, b2Transform(b2Vec2(0, -0.3), b2Rot(M_PI_2)));
        std::uniform_real_distribution<float> distribution(0, 1);
        const int particle_count = 20;
        const float step_subdivision = 1.f / particle_count / 10.f;
        glm::vec4 end_color = particle_color;
        end_color.a = 0;
        for (int i = 0; i < particle_count; ++i)
        {
            float random_number = agent->rng->next_float(distribution) - 0.5;
            b2Rot angle = b2Mul(edge_transform.q, b2Rot(random_number));
            Particle particle{
                glm::vec2(edge_transform.p.x + edge_transform.q.s * random_number, edge_transform.p.y - edge_transform.q.c * random_number),
                -glm::vec2(angle.c * 10, angle.s * 10),
                -i * step_subdivision,
                1,
                0.02,
                particle_color,
                end_color};
            render_data.particles.push_back(particle);
        }
    }

    return render_data;
}

void ThrusterModule::activate()
{
    active = true;
    b2Transform global_transform = get_global_transform();
    b2Vec2 velocity = b2Mul(global_transform.q, b2Vec2(0, 50));
    agent->rigid_body->body->ApplyForce(velocity, global_transform.p, true);
}

void ThrusterModule::update()
{
    active = false;
}
}