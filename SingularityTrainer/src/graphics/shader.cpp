#include <fstream>
#include <sstream>
#include <string>

#include "glad/glad.h"
#include "spdlog/spdlog.h"

#include "graphics/shader.h"

namespace SingularityTrainer
{
Shader::Shader(const std::string &vert_filepath, const std::string &frag_filepath)
{
    // Load shaders
    std::string vert_shader_source;
    std::string frag_shader_source;

    std::stringstream string_stream;

    std::ifstream vert_file_stream(vert_filepath);
    string_stream << vert_file_stream.rdbuf();
    vert_shader_source = string_stream.str();

    string_stream.str("");
    string_stream.clear();

    std::ifstream frag_file_stream(frag_filepath);
    string_stream << frag_file_stream.rdbuf();
    frag_shader_source = string_stream.str();

    // Compile shaders
    unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vert_shader_source);
    unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, frag_shader_source);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
}

Shader::~Shader() {}

void Shader::bind()
{
    glUseProgram(program);
}

void Shader::unbind()
{
    glUseProgram(0);
}

unsigned int Shader::compile_shader(unsigned int type, const std::string &source)
{
    // Compile shader
    unsigned int id = glCreateShader(type);
    const char *src_c = source.c_str();
    glShaderSource(id, 1, &src_c, nullptr);
    glCompileShader(id);

    // Check compilation result
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char message[length];
        glGetShaderInfoLog(id, length, &length, message);
        spdlog::error("Shader compile error: {}", std::string(message));
        glDeleteShader(id);
        return 0;
    }
    return id;
}
}