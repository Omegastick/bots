#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

#include "physics_system.h"
#include "environment/components/contact.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_type.h"
#include "environment/components/physics_world.h"
#include "environment/observers/destroy_body.h"
#include "misc/transform.h"

namespace ai
{
class ContactListener : public b2ContactListener
{
  private:
    entt::registry &registry;

  public:
    ContactListener(entt::registry &registry) : registry(registry) {}

    void BeginContact(b2Contact *contact)
    {
        auto fixture_a = contact->GetFixtureA();
        auto fixture_b = contact->GetFixtureB();
        const auto entity_a = static_cast<entt::registry::entity_type>(
            reinterpret_cast<uintptr_t>(fixture_a->GetUserData()));
        const auto entity_b = static_cast<entt::registry::entity_type>(
            reinterpret_cast<uintptr_t>(fixture_b->GetUserData()));

        const auto contact_entity = registry.create();
        registry.assign<ai::BeginContact>(contact_entity, entity_a, entity_b);
    }

    void EndContact(b2Contact *contact)
    {
        auto fixture_a = contact->GetFixtureA();
        auto fixture_b = contact->GetFixtureB();
        const auto entity_a = static_cast<entt::registry::entity_type>(
            reinterpret_cast<uintptr_t>(fixture_a->GetUserData()));
        const auto entity_b = static_cast<entt::registry::entity_type>(
            reinterpret_cast<uintptr_t>(fixture_b->GetUserData()));

        const auto contact_entity = registry.create();
        registry.assign<ai::EndContact>(contact_entity, entity_a, entity_b);
    }
};

void init_physics(entt::registry &registry)
{
    auto &world = registry.set<b2World>(b2Vec2{0, 0});
    registry.on_destroy<PhysicsBody>().connect<destroy_body>();
    auto &contact_listener = registry.set<ContactListener>(registry);
    world.SetContactListener(&contact_listener);
}

std::string type_to_string(PhysicsType::Type type)
{
    if (type == PhysicsType::Body)
    {
        return "Body";
    }
    else if (type == PhysicsType::Bullet)
    {
        return "Bullet";
    }
    else
    {
        return "Wall";
    }
}

void physics_system(entt::registry &registry, double delta_time)
{
    // Step world
    registry.ctx<b2World>().Step(static_cast<float>(delta_time), 3, 2);

    // Update transforms
    registry.view<PhysicsBody, Transform>().each([](auto &body, auto &transform) {
        const auto position = body.body->GetPosition();
        transform.set_position({position.x, position.y});
        const auto rotation = body.body->GetAngle();
        transform.set_rotation(rotation);
    });

    // Handle contacts
    registry.view<BeginContact>().each([&](const auto entity, auto &contact) {
        spdlog::debug("Begin contact");
        auto &physics_type_1 = registry.get<PhysicsType>(contact.entity_1);
        auto &physics_type_2 = registry.get<PhysicsType>(contact.entity_2);
        spdlog::debug("A: {} - B: {}",
                      type_to_string(physics_type_1.type),
                      type_to_string(physics_type_2.type));

        registry.destroy(entity);
    });
    registry.view<EndContact>().each([&](const auto entity, auto &contact) {
        spdlog::debug("End contact");
        auto &physics_type_1 = registry.get<PhysicsType>(contact.entity_1);
        auto &physics_type_2 = registry.get<PhysicsType>(contact.entity_2);
        spdlog::debug("A: {} - B: {}",
                      type_to_string(physics_type_1.type),
                      type_to_string(physics_type_2.type));
        registry.destroy(entity);
    });
}
}