#pragma once

#include "glm/vec4.hpp"

#include <string>
#include <unordered_map>

namespace SingularityTrainer
{
class Shader
{
  private:
    unsigned int compile_shader(unsigned int type, const std::string &source);
    std::unordered_map<std::string, int> uniform_location_cache;

  public:
    Shader(const std::string &vert_filepath, const std::string &frag_filepath);
    ~Shader();

    void bind() const;
    void unbind() const;

    void set_uniform_1i(const std::string &name, int value);
    void set_uniform_4f(const std::string &name, glm::vec4 value);
    int get_uniform_location(const std::string &name);

    unsigned int program;
};
}