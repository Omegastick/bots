#pragma once

#include "glm/vec4.hpp"

#include <string>

namespace SingularityTrainer
{
class Shader
{
  public:
    Shader(const std::string &vert_filepath, const std::string &frag_filepath);
    ~Shader();

    void bind() const;
    void unbind() const;
    // void set_uniform_4f(std::string &name, glm::vec4 value);

    unsigned int program;

  private:
    unsigned int compile_shader(unsigned int type, const std::string &source);
};
}