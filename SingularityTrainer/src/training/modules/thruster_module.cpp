#include <Box2D/Box2D.h>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "gui/colors.h"
#include "linear_particle_system.h"
#include "resource_manager.h"
#include "training/actions/activate_action.h"
#include "training/agents/iagent.h"
#include "training/modules/imodule.h"
#include "training/modules/thruster_module.h"

namespace SingularityTrainer
{
ThrusterModule::ThrusterModule(ResourceManager &resource_manager, b2Body &body, IAgent *agent, LinearParticleSystem *particle_system)
    : particle_system(particle_system)
{
    // Sprite
    resource_manager.load_texture("thruster_module", "images/thruster_module.png");
    sprite.setScale(0.01, 0.01);
    sprite.setTexture(*resource_manager.texture_store.get("thruster_module"));
    sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height / 2);

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
    module_links.push_back(ModuleLink(0, -0.167, 0, this));

    actions.push_back(std::make_unique<ActivateAction>(this));

    this->agent = agent;
}

ThrusterModule::~ThrusterModule() {}

void ThrusterModule::draw(sf::RenderTarget &render_target)
{
    IModule::draw(render_target);
}

void ThrusterModule::activate()
{
    b2Transform global_transform = get_global_transform();
    b2Vec2 velocity = b2Mul(global_transform.q, b2Vec2(0, -50));
    agent->rigid_body->body->ApplyForce(velocity, global_transform.p, true);

    // Spawn particles
    std::uniform_real_distribution<float> distribution(-0.5, 0.5);
    const int particle_count = 10;
    const float step_subdivision = 1.f / particle_count / 10.f;
    for (int i = 0; i < particle_count; ++i)
    {
        b2Rot angle = b2Mul(global_transform.q, b2Rot(M_PI_2));
        angle = b2Mul(angle, b2Rot(agent->rng->next_float(distribution)));
        Particle particle{
            global_transform.p,
            b2Vec2(angle.c * 10, angle.s * 10),
            cl_white,
            sf::Color::Transparent,
            0.5};
        particle_system->add_particle(particle, i * step_subdivision);
    }
}

void ThrusterModule::update() {}
}