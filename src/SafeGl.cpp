#include "SafeGl.h"
#include <iostream>
#include <fstream>

namespace
{
    using namespace SafeGl;

    std::string loadTextFile(std::filesystem::path path)
    {
        std::ifstream file(path);
        if (!file) throw std::invalid_argument("Unable to open file");

        std::ostringstream sstream;
        sstream << file.rdbuf();
        return std::move(sstream).str();
    }

    Shader compileShader(GLenum shader_type, std::string_view src)
    {
        Shader shader(glCreateShader(shader_type));
        const char* src_pointer = src.data();
        glShaderSource(shader, 1, &src_pointer, NULL);
        glCompileShader(shader);

        GLint result;
        GLint log_length;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0) {
            std::string log_message(log_length + 1, '\0');
            glGetShaderInfoLog(shader, log_length, NULL, log_message.data());
            std::clog << "[Shader compilation] : " << log_message << std::endl;
        }
        if (result != GL_TRUE) throw std::runtime_error("Shader compilation failed");

        return shader;
    }

    Program compileShaderProgram(std::string_view vertex_src, std::string_view fragment_src)
    {
        Shader vertex_shader(compileShader(GL_VERTEX_SHADER, vertex_src));
        Shader fragment_shader(compileShader(GL_FRAGMENT_SHADER, fragment_src));
        Program program(glCreateProgram());
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        GLint result;
        GLint log_length;
        glGetProgramiv(program, GL_LINK_STATUS, &result);
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0) {
            std::string log_message(log_length + 1, '\0');
            glGetProgramInfoLog(program, log_length, NULL, log_message.data());
            std::clog << "[Shader linking] : " << log_message << std::endl;
        }
        if (result != GL_TRUE) throw std::runtime_error("Shader linking failed");

        glDetachShader(program, vertex_shader);
        glDetachShader(program, fragment_shader);

        return program;
    }
}


namespace SafeGl
{
    Program loadAndCompileProgram(std::filesystem::path v, std::filesystem::path f)
    {
        return compileShaderProgram(loadTextFile(v), loadTextFile(f));
    }
}
