#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace SingularityTrainer
{
class Shader
{
  private:
    unsigned int compile_shader(unsigned int type, const std::string &source);
    mutable std::unordered_map<std::string, int> uniform_location_cache;

  public:
    Shader(const std::string &vert_filepath, const std::string &frag_filepath);
    ~Shader();

    void bind() const;
    void unbind() const;

    void set_uniform_1i(const std::string &name, int value) const;
    void set_uniform_1f(const std::string &name, float value) const;
    void set_uniform_2f(const std::string &name, glm::vec2 value) const;
    void set_uniform_4f(const std::string &name, glm::vec4 value) const;
    void set_uniform_mat4f(const std::string &name, const glm::mat4 &value) const;
    int get_uniform_location(const std::string &name) const;

    unsigned int program;
};
}