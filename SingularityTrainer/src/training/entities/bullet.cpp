#include <iostream>

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include "graphics/colors.h"
#include "graphics/idrawable.h"
#include "training/entities/bullet.h"
#include "training/icollidable.h"
#include "training/rigid_body.h"

namespace SingularityTrainer
{
Bullet::Bullet(b2Vec2 position, b2Vec2 velocity, b2World &world)
    : destroyed(false), life(10), last_position(b2Vec2_zero), particle_color(cl_white)
{
    sprite = std::make_unique<Sprite>("bullet");
    sprite->set_scale(glm::vec2(0.2, 0.2));
    sprite->set_origin(sprite->get_center());
    sprite->set_position(glm::vec2(position.x, position.y));

    rigid_body = std::make_unique<RigidBody>(b2_dynamicBody, position, world, this, RigidBody::ParentTypes::Bullet);

    b2CircleShape rigid_body_shape;
    rigid_body_shape.m_radius = 0.1;
    b2FixtureDef fixture_def;
    fixture_def.shape = &rigid_body_shape;
    fixture_def.density = 1;
    fixture_def.friction = 1;
    fixture_def.isSensor = false;
    rigid_body->body->CreateFixture(&fixture_def);
    rigid_body->body->SetBullet(true);

    rigid_body->body->ApplyForceToCenter(velocity, true);
}

Bullet::~Bullet() {}

RenderData Bullet::get_render_data(bool lightweight)
{
    RenderData render_data;

    if (!destroyed)
    {
        b2Vec2 position = rigid_body->body->GetPosition();

        // Body
        sprite->set_position(glm::vec2(position.x, position.y));
        render_data.sprites.push_back(*sprite);

        // Trail
        if (last_position.x != b2Vec2_zero.x || last_position.y != b2Vec2_zero.y)
        {
            Line trail;
            trail.points.push_back({position.x, position.y});
            trail.widths.push_back(0.1);
            trail.colors.push_back({1.0, 1.0, 1.0, 1.0});
            trail.points.push_back({last_position.x, last_position.y});
            trail.widths.push_back(0);
            trail.colors.push_back({1.0, 1.0, 1.0, 0.0});
            render_data.lines.push_back(trail);
        }

        last_position = position;
    }

    if (explosion_particles.size() > 1)
    {
        render_data.append(explosion_particles);
        explosion_particles.clear();
    }

    return render_data;
}

void Bullet::begin_contact(RigidBody *other)
{
    if (!destroyed && explosion_particles.size() == 0)
    {
        const int particle_count = 100;
        explosion_particles.reserve(particle_count);
        b2Vec2 transform = rigid_body->body->GetPosition();
        const float step_subdivision = 1.f / particle_count / 10.f;
        glm::vec4 end_color = particle_color;
        end_color.a = 0;
        for (int i = 0; i < particle_count; ++i)
        {
            Particle particle{
                glm::vec2(transform.x, transform.y),
                glm::diskRand(4.f),
                -i * step_subdivision,
                0.75,
                0.02,
                particle_color,
                end_color};
            explosion_particles.push_back(particle);
        }
    }
    destroyed = true;
}

void Bullet::end_contact(RigidBody *other) {}

void Bullet::update()
{
    --life;
    destroyed = destroyed || life <= 0;
}
}