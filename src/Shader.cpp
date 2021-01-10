#include "Shader.h"

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


std::map<std::string, Shader*> textureShaders;
std::map<std::string, Shader*> plainShaders;


Shader* Shader::getShader(const std::string& name, bool texture)
{
    std::map<std::string, Shader*>& cache = texture ? textureShaders : plainShaders;
    Shader* shader = cache[name];

    if (!shader) {
        std::string shaderPathBase = "shader/" + name;
        shader = new Shader(name, shaderPathBase + ".vs", shaderPathBase + (texture ? "_tex.fs" : ".fs"));
        cache[name] = shader;
    }

    return shader;
}

Shader::Shader(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
    : shaderName(name)
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
    if (setupDone) {
        return 0;
    }

    vertexShaderSource = loadSource(vertexShaderPath);
    fragmentShaderSource = loadSource(fragmentShaderPath);

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        return -1;
    }

    if (createProgram()) {
        return -1;
    }

    setupDone = true;
    return 0;
}

GLint Shader::getUniformLoc(const std::string& name)
{
    GLint vi = uniforms[name];
    if (!vi) {
        vi = glGetUniformLocation(id, name.c_str());
        uniforms[name] = vi;
        if (vi < 0) {
            std::cout << "SHADER::MISSING_UNIFORM: " << shaderName << " uniform=" << name << std::endl;
        }
    }
    return vi;
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

void Shader::setFloat3(const std::string& name, float v1, float v2, float v3)
{
    glUniform3f(getUniformLoc(name), v1, v2, v3);
}

void Shader::setVec3(const std::string& name, const glm::vec3& v)
{
    glUniform3f(getUniformLoc(name), v.x, v.y, v.z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& v)
{
    glUniform4f(getUniformLoc(name), v.x, v.y, v.z, v.w);
}

void Shader::setFloat(const std::string& name, float value)
{
    glUniform1f(getUniformLoc(name), value);
}

void Shader::setInt(const std::string& name, int value)
{
    glUniform1i(getUniformLoc(name), value);
}

void Shader::setIntArray(const std::string& name, int count, const GLint* values)
{
    glUniform1iv(getUniformLoc(name), count, values);
}

void Shader::setBool(const std::string& name, bool value)
{
    glUniform1i(getUniformLoc(name), (int)value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(getUniformLoc(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat)
{
    glUniformMatrix3fv(getUniformLoc(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat)
{
    glUniformMatrix2fv(getUniformLoc(name), 1, GL_FALSE, glm::value_ptr(mat));
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
