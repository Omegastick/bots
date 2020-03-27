#include <string>
#include <queue>

#include <Box2D/Box2D.h>
#include <doctest.h>
#include <entt/entt.hpp>
#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>

#include "serialize_body.h"
#include "environment/components/body.h"
#include "environment/components/module_link.h"
#include "environment/components/modules/base_module.h"
#include "environment/components/modules/gun_module.h"
#include "environment/components/modules/laser_sensor_module.h"
#include "environment/components/modules/module.h"
#include "environment/components/modules/square_hull.h"
#include "environment/components/modules/thruster_module.h"
#include "environment/components/name.h"
#include "environment/components/sensor_reading.h"
#include "environment/systems/clean_up_system.h"
#include "environment/utils/body_factories.h"
#include "environment/utils/body_utils.h"
#include "environment/utils/sensor_utils.h"
#include "graphics/colors.h"

namespace ai
{
static const std::string schema_version = "v1alpha8";

entt::entity deserialize_module(entt::registry &registry, const nlohmann::json &json)
{
    const auto entity = make_module(registry, json["type"]);

    if (json["type"] == "laser_sensor_module")
    {
        registry.get<EcsLaserSensorModule>(entity).laser_count = json["laser_count"];
        resize_sensor(registry, entity, json["laser_count"]);
    }

    return entity;
}

void deserialize_children(entt::registry &registry,
                          entt::entity module_entity,
                          const nlohmann::json &children_json)
{
    auto &module = registry.get<EcsModule>(module_entity);
    for (unsigned int i = 0; i < module.links; i++)
    {
        if (children_json[i].is_null())
        {
            continue;
        }

        const auto child_entity = deserialize_module(registry, children_json[i]);

        link_modules(registry,
                     module_entity,
                     i,
                     child_entity,
                     children_json[i]["parent_link_idx"]);
        deserialize_children(registry, child_entity, children_json[i]["links"]);
    }
}

entt::entity deserialize_body(entt::registry &registry, const nlohmann::json &json)
{
    if (json["schema"] != schema_version)
    {
        const auto error_message = fmt::format("Bad schema version: {}. Expected: {}",
                                               json["schema"].get<std::string>(),
                                               schema_version);
        throw std::runtime_error(error_message.c_str());
    }
    const auto body_entity = make_body(registry);
    const auto &body = registry.get<EcsBody>(body_entity);
    deserialize_children(registry, body.base_module, json["modules"]["links"]);

    registry.get<Name>(body_entity).name = json["name"];

    registry.get<ColorScheme>(body_entity).primary = {json["color_scheme"]["primary"][0],
                                                      json["color_scheme"]["primary"][1],
                                                      json["color_scheme"]["primary"][2],
                                                      json["color_scheme"]["primary"][3]};
    registry.get<ColorScheme>(body_entity).secondary = {json["color_scheme"]["secondary"][0],
                                                        json["color_scheme"]["secondary"][1],
                                                        json["color_scheme"]["secondary"][2],
                                                        json["color_scheme"]["secondary"][3]};

    return body_entity;
}

nlohmann::json serialize_module(const entt::registry &registry, entt::entity module_entity)
{
    nlohmann::json json;

    auto &module = registry.get<EcsModule>(module_entity);
    if (registry.has<EcsBaseModule>(module_entity))
    {
        json["type"] = "base_module";
    }
    else if (registry.has<EcsGunModule>(module_entity))
    {
        json["type"] = "gun_module";
    }
    else if (registry.has<EcsThrusterModule>(module_entity))
    {
        json["type"] = "thruster_module";
    }
    else if (registry.has<EcsLaserSensorModule>(module_entity))
    {
        json["type"] = "laser_sensor_module";
        json["laser_count"] = registry.get<EcsLaserSensorModule>(module_entity).laser_count;
    }
    else if (registry.has<EcsSquareHull>(module_entity))
    {
        json["type"] == "square_hull";
    }
    else
    {
        throw std::runtime_error("Trying to serialize invalid module");
    }

    entt::entity child_entity = module.first;
    entt::entity link_entity = module.first_link;
    for (unsigned int i = 0; i < module.links; i++)
    {
        const auto &link = registry.get<EcsModuleLink>(link_entity);
        if (link.child_link_index >= 0)
        {
            json["links"][i] = serialize_module(registry, child_entity);
            child_entity = registry.get<EcsModule>(child_entity).next;
            json["links"][i]["parent_link_idx"] = link.child_link_index;
        }
        else
        {
            json["links"][i] = nullptr;
        }
        link_entity = link.next;
    }

    return json;
}

nlohmann::json serialize_body(const entt::registry &registry, entt::entity body_entity)
{
    nlohmann::json json;
    json["schema"] = schema_version;
    json["name"] = registry.get<Name>(body_entity).name;

    const auto &body = registry.get<EcsBody>(body_entity);
    json["modules"] = serialize_module(registry, body.base_module);

    const auto color_scheme = registry.get<ColorScheme>(body_entity);
    json["color_scheme"]["primary"] =
        std::vector<float>(glm::value_ptr(color_scheme.primary),
                           glm::value_ptr(color_scheme.primary) + 4);
    json["color_scheme"]["secondary"] =
        std::vector<float>(glm::value_ptr(color_scheme.secondary),
                           glm::value_ptr(color_scheme.secondary) + 4);

    return json;
}

TEST_CASE("Body serialization")
{
    entt::registry registry;
    registry.set<b2World>(b2Vec2{0, 0});

    SUBCASE("deserialize_children() throws when given a bad module type")
    {
        const auto json = "{\"type\": \"bad_type\", \"links\": []}";
        const auto module_entity = make_base_module(registry);
        DOCTEST_CHECK_THROWS(deserialize_children(registry, module_entity, json));
    }

    SUBCASE("Bodies can be saved to Json and loaded back")
    {
        const auto body_entity = make_body(registry);
        const auto gun_module_entity = make_gun_module(registry);
        const auto thruster_module_entity = make_thruster_module(registry);
        auto &body = registry.get<EcsBody>(body_entity);
        link_modules(registry, body.base_module, 0, gun_module_entity, 1);
        link_modules(registry, gun_module_entity, 0, thruster_module_entity, 0);

        const auto json = serialize_body(registry, body_entity);
        const auto pretty_json = json.dump(2);
        INFO("Json: " << pretty_json);

        destroy_body(registry, body_entity);
        clean_up_system(registry);

        deserialize_body(registry, json);

        SUBCASE("Loaded Bodies have the same number of modules")
        {
            DOCTEST_CHECK(registry.view<EcsModule>().size() == 3);
        }
    }

    SUBCASE("Bodies with no modules are converted to Json correctly")
    {
        const auto body_entity = make_body(registry);

        const auto json = serialize_body(registry, body_entity);
        const auto pretty_json = json.dump(2);
        INFO("Json: " << pretty_json);

        for (const auto &link : json["modules"]["links"])
        {
            DOCTEST_CHECK(link.is_null());
        }
    }

    SUBCASE("Load Json errors on invalid schema version")
    {
        auto json = "{\"schema\": \"bad_schema\"}"_json;
        CHECK_THROWS(deserialize_body(registry, json));
    }
}

TEST_CASE("deserialize_module()")
{
    entt::registry registry;

    SUBCASE("Creates the right amount of lasers on a laser sensor module")
    {
        nlohmann::json json;
        json["type"] = "laser_sensor_module";
        json["laser_count"] = 5;
        const auto entity = deserialize_module(registry, json);

        clean_up_system(registry);
        DOCTEST_CHECK(registry.view<SensorReading>().size() == 5);
        DOCTEST_CHECK(registry.get<EcsLaserSensorModule>(entity).laser_count == 5);
        DOCTEST_CHECK(registry.get<Sensor>(entity).count == 5);
    }
}
}