#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <glad/gl.h>

#include "shader.h"

Shader::Shader(std::string vertex_path, std::string fragment_path)
{
    unsigned int vertexShader = load_shader(vertex_path, GL_VERTEX_SHADER);
    unsigned int fragmentShader = load_shader(fragment_path, GL_FRAGMENT_SHADER);

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint length = 0;
        glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &length);
        std::cout << "length: " << length << std::endl;
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "Linking shaders failed\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::Shader(std::string vertex_path, std::string geometry_path, std::string fragment_path)
{
    unsigned int vertexShader = load_shader(vertex_path, GL_VERTEX_SHADER);
    unsigned int geometryShader = load_shader(geometry_path, GL_GEOMETRY_SHADER);
    unsigned int fragmentShader = load_shader(fragment_path, GL_FRAGMENT_SHADER);

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, geometryShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    int success;
    char infoLog[512];
    glGetShaderiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint length = 0;
        glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &length);
        std::cout << "length: " << length << std::endl;
        glGetShaderInfoLog(ID, 512, NULL, infoLog);
        std::cout << "Linking shaders failed\n" << infoLog << std::endl;
    }

    // glValidateProgram(ID);
    // glGetShaderiv(ID, GL_VALIDATE_STATUS, &success);
    // if (!success)
    // {
    //     glGetShaderInfoLog(ID, 512, NULL, infoLog);
    //     std::cout << "Validating shaders failed\n" << infoLog << std::endl;
    // }


    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);
}

void Shader::use()
{
    glUseProgram(ID);
}

unsigned int Shader::load_shader(std::string pathh, GLenum type)
{
    std::ifstream t (pathh);
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string source_string = buffer.str();
    const char* source_code = source_string.c_str();
    unsigned int shader;
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source_code, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint length = 0;
        glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &length);
        std::cout << "length: " << length << std::endl;
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << pathh << " compilation failed\n" << infoLog << std::endl;
    }
    return shader;
}
