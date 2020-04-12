#include <Box2D/Box2D.h>
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

#include "physics_system.h"
#include "environment/components/contact.h"
#include "environment/components/physics_body.h"
#include "environment/components/physics_type.h"
#include "environment/components/physics_world.h"
#include "environment/observers/destroy_physics_body.h"
#include "environment/systems/contact_handlers/bullet_contact_handler.h"
#include "environment/systems/contact_handlers/hill_contact_handler.h"
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
        registry.emplace<ai::BeginContact>(contact_entity,
                                           std::array<entt::entity, 2>{entity_a, entity_b});
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
        registry.emplace<ai::EndContact>(contact_entity,
                                         std::array<entt::entity, 2>{entity_a, entity_b});
    }
};

void init_physics(entt::registry &registry)
{
    auto &world = registry.set<b2World>(b2Vec2{0, 0});
    registry.on_destroy<PhysicsBody>().connect<destroy_physics_body>();
    auto &contact_listener = registry.set<ContactListener>(registry);
    world.SetContactListener(&contact_listener);
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
    registry.view<BeginContact>().each([&](auto entity, auto &contact) {
        for (int i = 0; i < 2; i++)
        {
            const auto entity_1 = contact.entities[i];
            if (!registry.valid(entity_1))
            {
                return;
            }
            const auto entity_2 = contact.entities[(i + 1) % 2];
            if (!registry.valid(entity_2))
            {
                return;
            }
            const auto type_1 = registry.get<PhysicsType>(entity_1).type;
            const auto type_2 = registry.get<PhysicsType>(entity_2).type;
            switch (type_1)
            {
            case PhysicsType::Bullet:
                begin_bullet_contact(registry, entity_1, entity_2, type_2);
                break;
            case PhysicsType::Hill:
                begin_hill_contact(registry, entity_1, entity_2, type_2);
                break;
            default:
                break;
            }
        }
        registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(entity);
    });

    registry.view<EndContact>().each([&](auto entity, auto &contact) {
        for (int i = 0; i < 2; i++)
        {
            const auto entity_1 = contact.entities[i];
            if (!registry.valid(entity_1))
            {
                return;
            }
            const auto entity_2 = contact.entities[(i + 1) % 2];
            if (!registry.valid(entity_2))
            {
                return;
            }
            const auto type_1 = registry.get<PhysicsType>(entity_1).type;
            const auto type_2 = registry.get<PhysicsType>(entity_2).type;
            switch (type_1)
            {
            case PhysicsType::Hill:
                end_hill_contact(registry, entity_1, entity_2, type_2);
                break;
            default:
                break;
            }
        }
        registry.emplace_or_replace<entt::tag<"should_destroy"_hs>>(entity);
    });
}
}