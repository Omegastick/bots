#pragma once

#include <string>

#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <nlohmann/json_fwd.hpp>

namespace ai
{
class ColorScheme;
class IAudioEngine;
class Renderer;

class BuildEnv
{
  private:
    entt::registry registry;
    entt::entity body_entity;
    entt::entity cursor_entity;

  public:
    BuildEnv();

    void draw(Renderer &renderer, IAudioEngine &audio_engine);
    void forward(double delta_time);

    entt::entity create_module(const std::string &type);
    void delete_module(entt::entity module_entity);
    void move_module(entt::entity module_entity, glm::vec2 position, float rotation);
    void snap_module(entt::entity module_entity);
    bool link_module(entt::entity module_entity);

    entt::entity select_module(glm::vec2 position);
    void select_module(entt::entity module_entity);

    ColorScheme get_color_scheme() const;
    void set_color_scheme(const ColorScheme &color_scheme);

    std::string get_name() const;
    void set_name(const std::string &name);

    nlohmann::json serialize_body() const;
};
}