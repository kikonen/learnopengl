#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    this->vertexShaderPath = vertexShaderPath;
    this->fragmentShaderPath = fragmentShaderPath;
}

Shader::~Shader()
{
    glDeleteProgram(id);
    id = 0;
}

const void Shader::use()
{
    glUseProgram(id); 
}

int Shader::setup()
{
    vertexShaderSource = loadSource(vertexShaderPath);
    fragmentShaderSource = loadSource(fragmentShaderPath);

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        return -1;
    }

    if (createProgram()) {
        return -1;
    }

    return 0;
}

int Shader::createProgram() {
    int success;
    char infoLog[512];

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    {
        const char* src = vertexShaderSource.c_str();
        glShaderSource(vertexShader, 1, &src, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const char* src = fragmentShaderSource.c_str();
        glShaderSource(fragmentShader, 1, &src, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // link shaders
    id = glCreateProgram();
    glAttachShader(id, vertexShader);
    glAttachShader(id, fragmentShader);
    glLinkProgram(id);
    // check for linking errors
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return 0;
}

void Shader::setFloat3(std::string& name, float v1, float v2, float v3)
{
    glUniform3f(glGetUniformLocation(id, name.c_str()), v1, v2, v3);
}

void Shader::setFloat(std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setInt(std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::setBool(std::string& name, bool value)
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

/**
* Load shader file
*/
std::string Shader::loadSource(const std::string& path) {
    std::string src;
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(path);
        std::stringstream buf;
        buf << file.rdbuf();
        file.close();
        src = buf.str();
    } catch (std::ifstream::failure e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    std::cout << "\n== " << path << " ===\n" << src << "\n--------\n";

    return src;
}
